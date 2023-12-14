import { IGroup } from "../interface/IGroup";
import AbstractController from "./abstractController";
import express from 'express';

export default class GroupController extends AbstractController {
    constructor() {
        super('/groups');
    }

    public initRoutes() {
        this.router.get('/', (req: express.Request, res: express.Response, next: express.NextFunction) => this.getGroups(req, res, next));
    }

    /**
     * Get the group list
     */
    private async getGroups(request: express.Request, response: express.Response, next: express.NextFunction) {
        try {
            const grous: IGroup[] = await this._mysqlService.getGroups();
            response.status(200).json(grous);
        } catch (error: any) {
            this._loggerService.error(error);
            response.status(error.status ?? 500).json({
                message: error.message,
                status: error.status ?? 500
            });
        }
    }
}