import { Inject, Singleton } from "typescript-ioc";
import LoggerService from "./loggerService";
import { IPlayer } from "../interface/IPlayer";
import { ITop } from "../interface/ITop";
import { IGroup } from "../interface/IGroup";

@Singleton
export default class MysqlService {
    @Inject
    private _loggerService: LoggerService;

    async run() {
        this._loggerService.info("[Mysql Service] Running ");
    }

    async getPlayer(authid: string): Promise<IPlayer> {
        const player: IPlayer = {
            name: "Kriax",
            authid: "1234",
            picture: "",
            points: 1232,
            rank: 4
        }

        return player;
    }

    async getTop(groupReference: string): Promise<ITop> {
        return null;
    }

    async getGroups(): Promise<IGroup[]> {
        return null;
    }
}