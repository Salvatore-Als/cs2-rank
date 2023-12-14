export interface IPlayer {
    ignore_annouce: boolean;
    name: string;
    authid: string;
    points: number;
    rank: number | null;
    death_suicide: number;
    death_t: number;
    death_ct: number;
    bomb_planted: number;
    bomb_exploded: number;
    bomb_defused: number;
    kill_knife: number;
    kill_headshot: number;
    kill_t: number;
    kill_ct: number;
    teamkill_ct: number;
    teamkill_t: number;
    killassist_t: number;
    killassist_ct: number;
    metaData: IPlayerMetaData;
}

export interface IPlayerMetaData {
    missingRankPoints: number;
    headshot: number;
    knife: number;
    death: ITeamStats & {
        suicide: number;
    };
    kill: ITeamStats
    killAssist: ITeamStats;
    bomb: {
        planted: number,
        exploded: number,
        defused: number
    }
}

interface ITeamStats {
    total: number;
    ct: number;
    t: number;
}