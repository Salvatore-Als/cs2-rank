import { IPlayer } from "../interface/IPlayer";
import { ITopPlayer } from "../interface/ITop";
import AbstractController from "./abstractController";
import express from 'express';

export default class PlayerController extends AbstractController {
    constructor() {
        super('/players');
    }

    public initRoutes() {
        this.router.get('/top', (req: express.Request, res: express.Response, next: express.NextFunction) => this.getTop(req, res, next));
        this.router.get('/player/', (req: express.Request, res: express.Response, next: express.NextFunction) => this.getPlayers(req, res, next));
        this.router.get('/player/:id', (req: express.Request, res: express.Response, next: express.NextFunction) => this.getPlayer(req, res, next));
    }

    /**
     * Get the top
     * 
     * @param groupName Group
     * @param mapName   Map name, null get the global rank
     * @param page      Page to be displayed
     * @param limit     Number of player on the top
     */
    public async getTop(request: express.Request, response: express.Response, next: express.NextFunction) {
        try {
            const groupName: string = request.query?.group?.toString();
            const mapName: string = request.query?.map?.toString();
            const page: number = request.query?.page ? Number(request?.query?.map.toString()) : 0;
            const limit: number = request.query?.limit ? Number(request?.query?.limit.toString()) : 15;

            const { group, map } = await this.parsingGroupAndMap(groupName, mapName);
            const top: ITopPlayer[] = await this._mysqlService.getTop(group?.reference, page * limit, limit, map?.id);
            response.status(200).json(top);
        } catch (error: any) {
            this._loggerService.error(error);
            response.status(error.status ?? 500).json({
                message: error.message,
                status: error.status ?? 500
            });
        }
    }

    /**
     * Get the players
     * 
     * @param groupName Group
     * @param mapName   Map name, null get the global rank
     * @param page      Page to be displayed
     * @param limit     Number of player on the top
     */
    public async getPlayers(request: express.Request, response: express.Response, next: express.NextFunction) {
        try {
            const groupName: string = request.query?.group?.toString();
            const mapName: string = request.query?.map?.toString();
            const page: number = request.query?.page ? Number(request?.query?.map.toString()) : 0;
            const limit: number = request.query?.limit ? Number(request?.query?.limit.toString()) : 15;

            const { group, map } = await this.parsingGroupAndMap(groupName, mapName);
            const top: ITopPlayer[] = await this._mysqlService.getPlayers(group?.reference, page * limit, limit, map?.id);
            response.status(200).json(top);
        } catch (error: any) {
            this._loggerService.error(error);
            response.status(error.status ?? 500).json({
                message: error.message,
                status: error.status ?? 500
            });
        }
    }

    /**
     * Get a player 
     * 
     * @param userId        steamid64 or player name
     * @param groupName     Group
     * @param mapName       Map name, null get the global stats
     */
    public async getPlayer(request: express.Request, response: express.Response, next: express.NextFunction) {
        try {
            const userId: string = request.params?.id?.toString();
            const groupName: string = request.query?.group?.toString();
            const mapName: string = request.query?.map?.toString();

            let player: IPlayer = null;
            const { group, map } = await this.parsingGroupAndMap(groupName, mapName);

            const regex: RegExp = new RegExp(/^\d{17}$/);
            player = await this._mysqlService.getPlayer(regex.test(userId) ? "authid" : "name", userId, group?.reference, map?.id);

            if (!player) {
                throw new Error("Player not found");
            }

            response.status(200).json(player);
        } catch (error: any) {
            this._loggerService.error(error);
            response.status(error.status ?? 500).json({
                message: error.message,
                status: error.status ?? 500
            });
        }
    }
}