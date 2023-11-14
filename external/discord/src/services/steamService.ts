import { Singleton } from "typescript-ioc";
import { ISteamProfile } from "../interface/ISteamProfile";
import axios, { AxiosResponse } from 'axios';
import { XMLParser } from 'fast-xml-parser';

@Singleton
export default class SteamService {
    private _parser: XMLParser = new XMLParser();

    public async getProfile(authid: string): Promise<ISteamProfile> {
        const steamProfileUrl: string = `https://steamcommunity.com/profiles/${authid}?xml=1`;
        const steamProfile: AxiosResponse<any> = await axios.get(steamProfileUrl);
        const steamProfileXml = this._parser.parse(steamProfile.data);

        if (!steamProfileXml?.profile) {
            return {
                name: "INVALID STEAM NAME",
                picture: ""
            }
        }

        return {
            name: steamProfileXml.profile.steamID ? steamProfileXml.profile.steamID : steamProfileXml.profile.steamID64,
            picture: steamProfileXml?.profile?.avatarMedium
        } as ISteamProfile;
    }
}