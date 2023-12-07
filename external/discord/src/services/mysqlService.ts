import { Inject, Singleton } from "typescript-ioc";
import LoggerService from "./loggerService";
import { IPlayer } from "../interface/IPlayer";
import { ITopPlayer } from "../interface/ITop";
import { IGroup } from "../interface/IGroup";
import MysqlProvider from "../providers/mysqlProvider";
import { ILinkedAccout } from "../interface/ILinkedAccount";
import { IMap } from "../interface/IMap";

interface MysqlCountResult {
    count: number;
}

const RANK_SUM = `SUM(cr.points) AS points,
SUM(cr.death_suicide) AS death_suicide,
SUM(cr.death_t) AS death_t,
SUM(cr.death_ct) AS death_ct,
SUM(cr.bomb_planted) AS bomb_planted,
SUM(cr.bomb_exploded) AS bomb_exploded,
SUM(cr.bomb_defused) AS bomb_defused,
SUM(cr.kill_knife) AS kill_knife,
SUM(cr.kill_headshot) AS kill_headshot,
SUM(cr.kill_t) AS kill_t,
SUM(cr.kill_ct) AS kill_ct,
SUM(cr.teamkill_t) AS teamkill_t,
SUM(cr.teamkill_ct) AS teamkill_ct,
SUM(cr.killassist_t) AS killassist_t,
SUM(cr.killassist_ct) AS killassist_ct`;

@Singleton
export default class MysqlService {
    @Inject
    private _loggerService: LoggerService;

    @Inject
    private _mysqlProvider: MysqlProvider;

    async run() {
        if (process.env.MINIMUM_POINTS == null || process.env.MINIMUM_POINTS == undefined) {
            throw new Error("[Discord Service] Missing MINIMUM_POINTS variable on your environment file");
        }

        await this.createDiscordTable();
        this._loggerService.info("[Mysql Service] Running ");
    }

    async getRankBy(by: 'authid' | 'name', value: string, groupReference: string, mapId?: number): Promise<IPlayer> {
        this._loggerService.debug(`Rank by ${by}`);

        let points: number = null;
        let query: string = null; // Query to get the player
        let safeValue: any[] = [];

        switch (by) {
            case 'authid':
                points = await this.getPointByAuthId(value, groupReference, mapId);

                if (mapId != null) {
                    query = `SELECT cr.*, CAST(cu.authid AS CHAR) AS authid 
                             FROM cs2_rank_stats cr JOIN cs2_rank_users cu 
                             ON cr.user_id = cu.id WHERE cu.authid = ? AND cr.reference = ? AND cr.map = ?;`
                    safeValue = [value, groupReference, mapId];
                } else {
                    query = `SELECT CAST(cu.authid AS CHAR) AS authid,
                             ${RANK_SUM}
                             FROM cs2_rank_stats cr JOIN cs2_rank_users cu 
                             ON cr.user_id = cu.id WHERE cu.authid = ? AND cr.reference = ? GROUP BY cu.authid;`
                    safeValue = [value, groupReference];
                }
                break;
            case 'name':
                points = await this.getPointByName(value, groupReference, mapId);

                if (mapId != null) {
                    query = `SELECT cr.*, CAST(cu.authid AS CHAR) AS authid FROM cs2_rank_stats cr JOIN cs2_rank_users cu 
                             ON cr.user_id = cu.id WHERE cu.name LIKE ? AND cr.reference = ? AND cr.map = ?;`;
                    safeValue = [`%${value}%`, groupReference, mapId];
                } else {
                    query = `SELECT CAST(cu.authid AS CHAR) AS authid,
                             ${RANK_SUM}
                             FROM cs2_rank_stats cr JOIN cs2_rank_users cu 
                             ON cr.user_id = cu.id WHERE cu.name LIKE ? AND cr.reference = ? GROUP BY cu.authid;`
                    safeValue = [`%${value}%`, groupReference];
                }
                break;
            default:
                throw new Error(`Unsupported getRankBy ${by}`);
        }

        if (points == null) {
            return null;
        }

        const players: IPlayer[] = await this._mysqlProvider.query<IPlayer[]>(query, safeValue);
        let player: IPlayer = players[0];

        if (!player) {
            return null;
        }

        const minimumPoints: number = Number(process.env.MINIMUM_POINTS);
        player.rank = points < minimumPoints ? -1 : await this.getRank(points, groupReference, mapId);

        return player;
    }

    async getTop(groupReference: string, mapId?: number): Promise<ITopPlayer[]> {
        let query: string;
        let result: ITopPlayer[] = null;

        if (mapId != null) {
            query = `SELECT cu.name as name, CAST(cu.authid as CHAR) as authid, cr.points FROM cs2_rank_stats cr JOIN cs2_rank_users cu ON cr.user_id = cu.id WHERE points >= ? AND reference = ? AND map = ?
                    ORDER BY points DESC LIMIT 15;`;
            result = await this._mysqlProvider.query<ITopPlayer[]>(query, [Number(process.env.MINIMUM_POINTS), groupReference, mapId]);
        } else {
            query = `SELECT cu.name as name, CAST(cu.authid as CHAR) as authid, SUM(cr.points) AS total_points FROM cs2_rank_stats cr JOIN cs2_rank_users cu ON cr.user_id = cu.id 
                    WHERE cr.reference = ? GROUP BY cr.user_id HAVING SUM(cr.points) >= ? ORDER BY SUM(cr.points) DESC LIMIT 15;`;

            result = await this._mysqlProvider.query<ITopPlayer[]>(query, [groupReference, Number(process.env.MINIMUM_POINTS)]);
            result.map((value: ITopPlayer & { total_points: number }) => value.points = value.total_points);
        }

        return result;
    }

    async getGroups(): Promise<IGroup[]> {
        const query: string = 'SELECT id, reference, custom_name FROM cs2_rank_references';
        const result: IGroup[] = await this._mysqlProvider.query<IGroup[]>(query);
        return result;
    }

    public async getLinkedAccounts(): Promise<ILinkedAccout[]> {
        const query: string = "SELECT * FROM cs2_rank_accounts";
        const result: ILinkedAccout[] = await this._mysqlProvider.query<ILinkedAccout[]>(query);
        return result;
    }

    public async getMaps(): Promise<IMap[]> {
        const query: string = "SELECT * FROM cs2_rank_maps";
        const result: IMap[] = await this._mysqlProvider.query<IMap[]>(query);
        return result;
    }

    public async createLinkedAccount(authid: string, discordId: string): Promise<void> {
        const query: string = "INSERT INTO cs2_rank_accounts (authid, discordid) VALUES (?, ?)";
        const result: void = await this._mysqlProvider.query<void>(query, [authid, discordId]);
        return result;
    }

    public async getMapByName(name: string): Promise<IMap> {
        const query: string = "SELECT * FROM cs2_rank_maps WHERE name LIKE ?";
        const maps: IMap[] = await this._mysqlProvider.query<IMap[]>(query, [`%${name}%`]);

        if (!maps?.length) {
            return null;
        }

        return maps[0];
    }

    private async createDiscordTable(): Promise<void> {
        return new Promise(async (resolve, reject) => {
            const query: string = "CREATE TABLE cs2_rank_accounts (id INT AUTO_INCREMENT PRIMARY KEY, authid VARCHAR(255) UNIQUE, discordid VARCHAR(255) UNIQUE)";
            this._mysqlProvider.query<IGroup[]>(query)
                .then(() => {
                    resolve();
                }).catch((error: any) => {
                    error.code == 'ER_TABLE_EXISTS_ERROR' ? resolve() : reject(error);
                });
        });
    }

    private async getRank(points: number, groupReference: string, mapId?: number): Promise<number> {
        let query: string;
        let results: MysqlCountResult[];

        if (mapId != null) {
            this._loggerService.debug(`[Mysql Service] Get rank by mapId ${mapId}`);
            query = "SELECT COUNT(*) as count FROM cs2_rank_stats WHERE points >= ? AND reference = ? AND map = ?";
            results = await this._mysqlProvider.query<MysqlCountResult[]>(query, [points ?? 10, groupReference, mapId]);
        } else {
            this._loggerService.debug(`[Mysql Service] Get rank`);
            query = `SELECT COUNT(DISTINCT user_id) as count FROM (SELECT user_id, SUM(points) as total_points FROM cs2_rank_stats 
            WHERE reference = ? GROUP BY user_id) AS subquery WHERE total_points >= ?;`;
            results = await this._mysqlProvider.query<MysqlCountResult[]>(query, [groupReference, points ?? 10]);
        }

        const result: MysqlCountResult = results[0];
        return result.count;
    }


    private async getPointByAuthId(authid: string, groupReference: string, mapId?: number): Promise<number> {
        let query: string = null;
        let result: IPlayer & { total_points?: number } = null;

        if (mapId != null) {
            query = "SELECT cr.points FROM cs2_rank_stats cr JOIN cs2_rank_users cu ON cr.user_id = cu.id WHERE cu.authid = ? AND cr.reference = ? AND cr.map = ?;";
            result = await this._mysqlProvider.query<IPlayer & { total_points: number }>(query, [authid, groupReference]);
            return result?.points;
        } else {
            query = "SELECT SUM(cr.points) as total_points FROM cs2_rank_stats cr JOIN cs2_rank_users cu ON cr.user_id = cu.id WHERE cu.authid = ? AND cr.reference = ?;";
            result = await this._mysqlProvider.query<IPlayer & { total_points: number }>(query, [authid, groupReference]);
            return result?.total_points;
        }
    }

    private async getPointByName(name: string, groupReference: string, mapId?: number): Promise<number> {
        let query: string = null;
        let players: IPlayer[] = [];
        let key: 'total_points' | 'points' = null;

        if (mapId != null) {
            query = "SELECT points FROM cs2_rank_stats cr JOIN cs2_rank_users cu ON cr.user_id = cu.id WHERE cu.name LIKE ? AND cr.reference = ? AND cr.map = ?;";
            players = await this._mysqlProvider.query<IPlayer[]>(query, [`%${name}%`, groupReference, mapId]);
            key = 'points';
        } else {
            query = "SELECT SUM(cr.points) as total_points FROM cs2_rank_stats cr JOIN cs2_rank_users cu ON cr.user_id = cu.id WHERE cu.name LIKE ? AND cr.reference = ?;";
            players = await this._mysqlProvider.query<IPlayer[]>(query, [`%${name}%`, groupReference]);
            key = 'total_points';
        }

        if (!players?.length) {
            return null;
        }

        return players[0][key];
    }
}