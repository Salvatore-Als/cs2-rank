import { Inject, Singleton } from "typescript-ioc";
import LoggerService from "./loggerService";
import { IPlayer } from "../interface/IPlayer";
import { ITopPlayer } from "../interface/ITop";
import { IGroup } from "../interface/IGroup";
import MysqlProvider from "../providers/mysqlProvider";
import { ILinkedAccout } from "../interface/ILinkedAccount";

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

    async getRankByAuthid(authid: string, groupReference: string): Promise<IPlayer> {
        this._loggerService.debug("Rank by authid");

        const points: number = await this.getPointByAuthId(authid, groupReference);

        if (points == null) {
            return null;
        }

        let query: string = "SELECT *, CAST(authid AS CHAR) AS authid FROM verygames_rank_users WHERE authid = ? AND server = ? ORDER BY points DESC";
        let players: IPlayer[] = await this._mysqlProvider.query<IPlayer[]>(query, [authid, groupReference]);
        let player: IPlayer = players[0];

        if (!player) {
            return null;
        }

        const minimumPoints: number = Number(process.env.MINIMUM_POINTS);
        player.rank = points < minimumPoints ? -1 : await this.getRank(points, groupReference);

        return player;
    }

    async getRankByName(name: string, groupReference: string): Promise<IPlayer> {
        this._loggerService.debug("Rank by name");

        const points: number = await this.getPointByName(name, groupReference);

        if (points == null) {
            return null;
        }

        let query: string = "SELECT *, CAST(authid AS CHAR) AS authid FROM verygames_rank_users WHERE name LIKE ? AND server = ? ORDER BY points DESC";
        let players: IPlayer[] = await this._mysqlProvider.query<IPlayer[]>(query, [`%${name}%`, groupReference]);
        let player: IPlayer = players[0];

        if (!player) {
            return;
        }

        const minimumPoints: number = Number(process.env.MINIMUM_POINTS);
        player.rank = points < minimumPoints ? -1 : await this.getRank(points, groupReference);

        return player;
    }

    async getTop(groupReference: string): Promise<ITopPlayer[]> {
        const query: string = "SELECT CAST(authid AS CHAR) AS authid, name, points FROM verygames_rank_users WHERE points >= ? AND server = ? ORDER BY points DESC";
        const result: ITopPlayer[] = await this._mysqlProvider.query<ITopPlayer[]>(query, [Number(process.env.MINIMUM_POINTS), groupReference]);
        return result;
    }

    async getGroups(): Promise<IGroup[]> {
        const query: string = 'SELECT id, reference, custom_name FROM verygames_rank_servers';
        const result: IGroup[] = await this._mysqlProvider.query<IGroup[]>(query);
        return result;
    }

    public async getLinkedAccounts(): Promise<ILinkedAccout[]> {
        const query: string = "SELECT * FROM verygames_rank_accounts";
        const result: ILinkedAccout[] = await this._mysqlProvider.query<ILinkedAccout[]>(query);
        return result;
    }

    public async createLinkedAccount(authid: string, discordId: string): Promise<void> {
        const query: string = "INSERT INTO verygames_rank_accounts (authid, discordid) VALUES (?, ?)";
        const result: void = await this._mysqlProvider.query<void>(query, [authid, discordId]);
        return result;
    }

    private async createDiscordTable(): Promise<void> {
        return new Promise(async (resolve, reject) => {
            const query: string = "CREATE TABLE verygames_rank_accounts (id INT AUTO_INCREMENT PRIMARY KEY, authid VARCHAR(255) UNIQUE, discordid VARCHAR(255) UNIQUE)";
            this._mysqlProvider.query<IGroup[]>(query)
                .then(() => {
                    resolve();
                }).catch((error: any) => {
                    error.code == 'ER_TABLE_EXISTS_ERROR' ? resolve() : reject(error);
                });
        });
    }

    private async getRank(points: number, groupReference: string): Promise<number> {
        const query: string = "SELECT COUNT(*) as count FROM verygames_rank_users WHERE points > ? AND server = ?";
        const results: MysqlCountResult[] = await this._mysqlProvider.query<MysqlCountResult[]>(query, [points ?? 10, groupReference]);
        const result: MysqlCountResult = results[0];

        return result.count;
    }

    private async getPointByAuthId(authid: string, groupReference: string): Promise<number> {
        const query: string = "SELECT points FROM verygames_rank_users WHERE authid = ? AND server = ?";
        const result: IPlayer = await this._mysqlProvider.query<IPlayer>(query, [authid, groupReference]);

        return result[0].points;
    }

    private async getPointByName(name: string, groupReference: string): Promise<number> {
        const query: string = "SELECT points FROM verygames_rank_users WHERE name LIKE ? AND server = ?";
        const players: IPlayer[] = await this._mysqlProvider.query<IPlayer[]>(query, [name, groupReference]);

        return players[0]?.points;
    }
}