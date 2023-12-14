import { Singleton } from "typescript-ioc";
import { IMap } from "../interface/IMap";
import AbstractController from "./abstractController";
import express from 'express';

export default class MapController extends AbstractController {
    constructor() {
        super('/maps');
    }

    public initRoutes() {
        this.router.get('/', (req: express.Request, res: express.Response, next: express.NextFunction) => this.getMaps(req, res, next));
    }

    /**
     * Get the map list
     */
    public async getMaps(request: express.Request, response: express.Response, next: express.NextFunction) {
        try {
            const maps: IMap[] = await this._mysqlService.getMaps();
            response.status(200).json(maps);
        } catch (error: any) {
            this._loggerService.error(error);
            response.status(error.status ?? 500).json({
                message: error.message,
                status: error.status ?? 500
            });
        }
    }
}