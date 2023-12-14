import { IGroup } from "../interface/IGroup";
import { IMap } from "../interface/IMap";
import LoggerService from "../services/loggerService";
import MysqlService from "../services/mysqlService";
import express, { Router, Request, Response, NextFunction } from "express";
import { Inject } from 'typescript-ioc';
import { CError } from "../utils/error";

export default abstract class AbstractController {
    @Inject
    protected _loggerService: LoggerService

    @Inject
    protected _mysqlService: MysqlService

    protected router: express.Router;

    public path: string;

    constructor(path: string) {
        this.router = Router();
        this.router.use((request: Request, response: Response, next: NextFunction) => {
            response.setHeader('Access-Control-Allow-Origin', '*');
            response.setHeader('Access-Control-Allow-Methods', 'GET, POST, OPTIONS, DELETE, PATCH');
            response.setHeader('Access-Control-Allow-Headers', 'Access-Control-Allow-Origin,Access-Control-Allow-Methods');
            next()
        });

        this.path = path;
        this._loggerService.debug(`[AbstractController] Register new controller with path ${path}`);
    }

    protected abstract initRoutes(): void;

    protected async parsingGroupAndMap(groupName: string, mapName: string): Promise<{ group: IGroup; map: IMap; }> {
        if (!groupName) {
            throw new CError("Group can not be null", 401);
        }

        let group: IGroup = null;
        let map: IMap = null;

        group = await this._mysqlService.getGroupByName(groupName);
        if (!group) {
            throw new CError("Group not found", 404);
        }

        if (mapName) {
            map = await this._mysqlService.getMapByName(mapName);
            if (!map) {
                throw new CError("Map not found", 404);
            }
        }

        return { group, map };
    }
}