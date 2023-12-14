import { Inject } from "typescript-ioc";
import LoggerService from "./services/loggerService";
import MysqlService from "./services/mysqlService";
import MysqlProvider from "./providers/mysqlProvider";
import GroupController from "./controllers/groupController";
import MapController from "./controllers/mapController";
import PlayerController from "./controllers/playerController";
import express from "express";
import cors from 'cors';
import bodyParser from "body-parser";
import { CError } from "./utils/error";

export default class App {
    @Inject
    private _mysqlProvider: MysqlProvider;

    @Inject
    private _loggerService: LoggerService

    @Inject
    private _mysqlService: MysqlService;

    @Inject
    private _groupController: GroupController;

    @Inject
    private _mapController: MapController

    @Inject
    private _playerController: PlayerController

    private _app: any = null;

    constructor() {

    }

    public async run(): Promise<void> {
        try {
            const port: number = Number(process.env.PORT);
            if (!port) {
                throw new CError("Missing PORT variable on your environment file", 500);
            }

            this._app = express();

            // Don't touch the running order !

            this._loggerService.run();

            await this._mysqlProvider.run();
            await this._mysqlService.run();

            this._app.use(
                bodyParser.urlencoded({
                    extended: true,
                })
            );
            this._app.use(bodyParser.json());
            this._app.use(express.json());
            this._app.use(express.urlencoded());
            this._app.use(
                cors({
                    allowedHeaders: ['Access-Control-Allow-Origin', 'Access-Control-Allow-Methods', 'Content-Type'],
                    exposedHeaders: ['Access-Control-Allow-Origin', 'Access-Control-Allow-Methods', 'Content-Type'],
                    methods: ["GET", "HEAD", "PUT", "PATCH", "POST", "DELETE", "OPTIONS"],
                    origin: "*"
                })
            );

            this.initRoutes();

            this._app.listen(port, () => {
                this._loggerService.info(`[APP] Running on port ${port}`);
            });
        } catch (error: any) {
            this._loggerService.error("[FATAL ERROR] " + error);
        }
    }

    private initRoutes(): void {
        let controllers: any[] = [
            this._groupController,
            this._playerController,
            this._mapController
        ]

        controllers.forEach((controller: any) => {
            controller.initRoutes();
            this._app.use(controller.path, controller.router);
        });
    }
}