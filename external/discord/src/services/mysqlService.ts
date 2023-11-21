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

    async getRankByAuthid(authid: string, groupReference: string, mapId?: number): Promise<IPlayer> {
        this._loggerService.debug("Rank by authid");

        const points: number = await this.getPointByAuthId(authid, groupReference);

        if (points == null) {
            return null;
        }

        const query: string = "SELECT *, CAST(authid AS CHAR) AS authid FROM cs2_rank_users WHERE authid = ? AND reference = ? ORDER BY points DESC";
        const players: IPlayer[] = await this._mysqlProvider.query<IPlayer[]>(query, [authid, groupReference]);

        let player: IPlayer = players[0];

        if (!player) {
            return null;
        }

        const minimumPoints: number = Number(process.env.MINIMUM_POINTS);
        player.rank = points < minimumPoints ? -1 : await this.getRank(points, groupReference, mapId);

        return player;
    }

    async getRankByName(name: string, groupReference: string, mapId?: number): Promise<IPlayer> {
        this._loggerService.debug("Rank by name");

        const points: number = await this.getPointByName(name, groupReference);

        if (points == null) {
            return null;
        }

        const query: string = "SELECT *, CAST(authid AS CHAR) AS authid FROM cs2_rank_users WHERE name LIKE ? AND reference = ? ORDER BY points DESC";
        const players: IPlayer[] = await this._mysqlProvider.query<IPlayer[]>(query, [`%${name}%`, groupReference]);

        let player: IPlayer = players[0];

        if (!player) {
            return;
        }

        const minimumPoints: number = Number(process.env.MINIMUM_POINTS);
        player.rank = points < minimumPoints ? -1 : await this.getRank(points, groupReference, mapId);

        return player;
    }

    async getTop(groupReference: string, mapId?: number): Promise<ITopPlayer[]> {
        let query: string;
        let result: ITopPlayer[] = null;

        if (mapId != null) {
            query = "SELECT CAST(authid AS CHAR) AS authid, name, points FROM cs2_rank_users WHERE points >= ? AND reference = ? AND `map` = ? ORDER BY points DESC";
            result = await this._mysqlProvider.query<ITopPlayer[]>(query, [Number(process.env.MINIMUM_POINTS), groupReference, mapId]);
        } else {
            query = "SELECT CAST(authid AS CHAR) AS authid, name, points FROM cs2_rank_users WHERE points >= ? AND reference = ? ORDER BY points DESC";
            result = await this._mysqlProvider.query<ITopPlayer[]>(query, [Number(process.env.MINIMUM_POINTS), groupReference]);
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
            query = "SELECT COUNT(*) as count FROM cs2_rank_users WHERE points > ? AND reference = ? AND `map` = ?";
            results = await this._mysqlProvider.query<MysqlCountResult[]>(query, [points ?? 10, groupReference, mapId]);
        } else {
            this._loggerService.debug(`[Mysql Service] Get rank`);
            query = "SELECT COUNT(*) as count FROM cs2_rank_users WHERE reference = ? GROUP BY authid HAVING SUM(points) > ?";
            results = await this._mysqlProvider.query<MysqlCountResult[]>(query, [groupReference, points ?? 10]);
        }

        const result: MysqlCountResult = results[0];
        return result.count;
    }

    private async getPointByAuthId(authid: string, groupReference: string): Promise<number> {
        const query: string = "SELECT points FROM cs2_rank_users WHERE authid = ? AND reference = ?";
        const result: IPlayer = await this._mysqlProvider.query<IPlayer>(query, [authid, groupReference]);

        if (!result) {
            return null;
        }

        return result[0].points;
    }

    private async getPointByName(name: string, groupReference: string): Promise<number> {
        const query: string = "SELECT points FROM cs2_rank_users WHERE name LIKE ? AND reference = ?";
        const players: IPlayer[] = await this._mysqlProvider.query<IPlayer[]>(query, [`%${name}%`, groupReference]);

        if (!players?.length) {
            return null;
        }

        return players[0]?.points;
    }
}