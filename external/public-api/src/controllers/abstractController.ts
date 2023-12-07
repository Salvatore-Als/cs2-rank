import { IGroup } from "../interface/IGroup";
import { IMap } from "../interface/IMap";
import LoggerService from "../services/loggerService";
import MysqlService from "../services/mysqlService";
import express, { Router, Request, Response, NextFunction } from "express";
import { Inject } from 'typescript-ioc';

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

    protected parseError(error: any): any {
        return error.code ? error.code : error
    }

    protected async parsingGroupAndMap(groupName: string, mapName: string): Promise<{ group: IGroup; map: IMap; }> {
        if (!groupName) {
            throw new Error("Group can not be null");
        }

        let group: IGroup = null;
        let map: IMap = null;

        group = await this._mysqlService.getGroupByName(groupName);
        if (!group) {
            throw new Error("Group not found");
        }

        if (mapName) {
            map = await this._mysqlService.getMapByName(mapName);
            if (!map) {
                throw new Error("Map not found");
            }
        }

        return { group, map };
    }
}