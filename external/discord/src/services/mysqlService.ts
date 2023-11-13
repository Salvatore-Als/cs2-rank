import { Inject, Singleton } from "typescript-ioc";
import LoggerService from "./loggerService";
import { IPlayer } from "../interface/IPlayer";
import { ITopPlayer } from "../interface/ITop";
import { IGroup } from "../interface/IGroup";
import MysqlProvider from "../providers/mysqlProvider";

@Singleton
export default class MysqlService {
    @Inject
    private _loggerService: LoggerService;

    @Inject
    private _mysqlProvider: MysqlProvider;

    async run() {
        this._loggerService.info("[Mysql Service] Running ");
    }

    async getRankByAuthid(authid: string): Promise<IPlayer> {
        const player: IPlayer = {
            name: "Kriax",
            authid: "1234",
            picture: "",
            points: 1232,
            rank: 4
        }

        return player;
    }

    async getRankByName(name: string): Promise<IPlayer> {
        const selectQuery = 'SELECT * FROM votre_table WHERE id = ?';
        const selectResult = await this._mysqlProvider.query(selectQuery, ['name']);

        return null;
    }

    async getTop(groupReference: string): Promise<ITopPlayer[]> {
        const query: string = 'SELECT authid, name, points FROM verygames_rank_users WHERE points >= ? AND server = ? ORDER BY points DESC';
        const result: ITopPlayer[] = await this._mysqlProvider.query<ITopPlayer>(query, [process.env.MINIMUM_POINT ?? 10, groupReference]);
        return result;
    }

    async getGroups(): Promise<IGroup[]> {
        const query: string = 'SELECT id, reference, custom_name FROM verygames_rank_servers';
        const result: IGroup[] = await this._mysqlProvider.query<IGroup>(query);
        return result;
    }
}