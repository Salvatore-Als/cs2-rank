import { Inject, Singleton } from "typescript-ioc";
import LoggerService from "../services/loggerService";

@Singleton
export default class MysqlProvider {
    @Inject
    private _loggerService: LoggerService;

    async run() {
        this._loggerService.info("[Mysql Provider] Running ");
    }
}