import { Inject, Singleton } from "typescript-ioc";
import LoggerService from "../services/loggerService";
import mysql2, { Connection } from "mysql2/promise";

@Singleton
export default class MysqlProvider {
    @Inject
    private _loggerService: LoggerService;

    async run() {
        const start = new Date().getTime();

        const firstConnect: Connection = await this._createConnection();
        firstConnect.destroy();

        this._loggerService.info("[Mysql Provider] Running - " + (new Date().getTime() - start) / 1000);
    }

    private async _createConnection(): Promise<Connection> {
        return await mysql2.createConnection({
            host: process.env.MYSQL_HOST,
            user: process.env.MYSQL_USER,
            database: process.env.MYSQL_DATABASE,
            password: process.env.MYSQL_PASSWORD,
            port: Number(process.env.MYSQL_PORT) ?? 3306
        });
    }

    async query<T>(query: string, values?: any): Promise<T> {
        this._loggerService.debug(`[Mysql Provider] Executing query\n- Query: ${query}\n- Parameters: ${values}`);
        const connect: Connection = await this._createConnection();
        const [result] = await connect.execute(query, values);
        const mysqlResult: T = result as T;

        connect.destroy();

        return mysqlResult;
    }
}