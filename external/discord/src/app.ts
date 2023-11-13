import { Inject } from "typescript-ioc";
import DiscordProvider from "./providers/discordProvider";
import LoggerService from "./services/loggerService";
import DiscordService from "./services/discordService";
import MysqlService from "./services/mysqlService";
import MysqlProvider from "./providers/mysqlProvider";

export default class App {
    @Inject
    private _discordProvider: DiscordProvider;

    @Inject
    private _mysqlProvider: MysqlProvider;

    @Inject
    private _loggerService: LoggerService

    @Inject
    private _discordService: DiscordService

    @Inject
    private _mysqlService: MysqlService;

    constructor() {

    }

    public async run(): Promise<void> {
        try {
            this._loggerService.run();

            // Don't touch the running order !
            await this._mysqlProvider.run();
            await this._mysqlService.run();

            await this._discordProvider.run();
            this._discordService.run();

            this._loggerService.info("[APP] Running")
        } catch (error: any) {
            this._loggerService.error("[FATAL ERROR] " + error);
        }
    }
}