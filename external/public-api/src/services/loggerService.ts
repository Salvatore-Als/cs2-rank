import { Singleton } from "typescript-ioc";
import winston from "winston";

@Singleton
export default class LoggerService {
    private _logger: winston.Logger;

    constructor() {


    }

    async run() {
        const colorizer = winston.format.colorize();

        this._logger = winston.createLogger({
            format: winston.format.combine(
                winston.format.timestamp({
                    format: 'YYYY-MM-DD HH:mm:ss.SSSS'
                }),
                winston.format.simple(),
                winston.format.printf(msg =>
                    `${msg.timestamp} - ` + colorizer.colorize(msg.level, `${msg.level}: ${msg.message}`)
                )
            ),
            transports: [
                new winston.transports.Console({
                    level: 'debug'
                })
            ]
        });

        this.info("[Logger Service] Running")
    }

    debug(msg: any) {
        if (process.env.DEVELOPMENT != "true") {
            return;
        }

        if (this._logger && msg) {
            this._logger.debug(msg);
        }
    }

    info(msg: any) {
        if (this._logger && msg) {
            this._logger.info(msg);
        }
    }

    error(msg: any) {
        if (this._logger && msg) {
            this._logger.error(msg);
        }
    }

    warn(msg: any) {
        if (this._logger && msg) {
            this._logger.warn(msg);
        }
    }
}