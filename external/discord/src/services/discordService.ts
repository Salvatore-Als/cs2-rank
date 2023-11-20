import * as Discord from 'discord.js';
import { Inject, Singleton } from 'typescript-ioc';
import { EmptyString, IColor } from '../utils/const';
import _ from 'lodash';
import LoggerService from './loggerService';
import DiscordProvider from '../providers/discordProvider';
import moment from 'moment';
import MysqlService from './mysqlService';
import { ITopPlayer } from '../interface/ITop';
import { IPlayer, IPlayerMetaData } from '../interface/IPlayer';
import TranslationService from './translationService';
import { ITranslateKey } from '../interface/ITranslate';
import { ILinkedAccout } from '../interface/ILinkedAccount';
import SteamService from './steamService';
import { ISteamProfile } from '../interface/ISteamProfile';
import { IMap } from '../interface/IMap';

interface RequestLink {
    authid: string;
    name: string;
    expire: moment.Moment;
}

const embededGhostField: any = {
    name: EmptyString,
    value: EmptyString,
    inline: true
}

@Singleton
export default class DiscordService {
    @Inject
    private _discordProvider!: DiscordProvider;

    @Inject
    private _loggerService: LoggerService;

    @Inject
    private _mysqlService: MysqlService;

    @Inject
    private _translationService: TranslationService;

    @Inject
    private _steamService: SteamService;

    private _linkedAccounts: Map<string, string> = new Map();
    private _requestLinkedAccounts: Map<string, RequestLink> = new Map();
    private _client: Discord.Client = null;

    constructor() {

    }

    public async run(): Promise<void> {
        this._client = this._discordProvider.client;
        this._subscribe();

        await this._getLinkedAccounts();

        this._loggerService.info("[Discord Service] Running ");
    }

    private async _getLinkedAccounts(): Promise<void> {
        const linkedAccounts: ILinkedAccout[] = await this._mysqlService.getLinkedAccounts();
        linkedAccounts.map((linkedAccount: ILinkedAccout) => this._linkedAccounts.set(linkedAccount.discordid, linkedAccount.authid));

        this._loggerService.debug(`[Discord Service] Found ${linkedAccounts?.length} linked accounts`);
    }

    private _subscribe(): void {
        this._client.on('interactionCreate', async (interaction: Discord.Interaction<Discord.CacheType>) => {
            try {
                if (interaction.isChatInputCommand()) {
                    switch (interaction.commandName) {
                        case this._translationService.translate(ITranslateKey.Command_Rank):
                            await this._sendRank(interaction);
                            break;
                        case this._translationService.translate(ITranslateKey.Command_Top):
                            await this._sendTop(interaction);
                            break;
                        case this._translationService.translate(ITranslateKey.Command_Link):
                            await this._sendLink(interaction);
                            break;
                        case this._translationService.translate(ITranslateKey.Command_Map):
                            await this._sendMaps(interaction);
                            break;
                    }

                    return;
                }
            } catch (error: any) {
                this._loggerService.error(`[Discord Provider] ${error.stack} `);
            }
        });
    }

    private async _sendMaps(interaction: Discord.ChatInputCommandInteraction<Discord.CacheType>): Promise<void> {
        try {
            const maps: IMap[] = await this._mysqlService.getMaps();
            const chunks: Array<IMap[]> = _.chunk(maps, 15);

            if (!maps?.length || !chunks?.length) {
                this._sendError(interaction, this._translationService.translate(ITranslateKey.Sentence_PlayerNotFound));
                return;
            }

            const embeds: Discord.EmbedBuilder[] = [];

            for (let chunk of chunks) {
                let content: string = "";
                chunk.map((map: IMap) => content = content + `\n` + this._translationService.translate(ITranslateKey.Stats_Map, map.name));

                const embeded: Discord.EmbedBuilder = new Discord.EmbedBuilder();
                embeded
                    .setTitle(this._translationService.translate(ITranslateKey.Title_Maps))
                    .setColor(IColor.Success)
                    .setDescription(content);

                embeds.push(embeded);
            }

            await interaction.reply({ embeds: embeds, ephemeral: true });
        } catch (error: any) {
            this._sendError(interaction, this._translationService.translate(ITranslateKey.Sentence_ActionNotPossible));
            this._loggerService.error(error);
        }
    }

    private async _sendRank(interaction: Discord.ChatInputCommandInteraction<Discord.CacheType>): Promise<void> {
        try {
            const userId: string = interaction.user.id;
            const group: string = interaction.options.getString('group', true);
            const mapName: string = interaction.options.getString('map', false);
            let search: string = interaction.options.getString('player', false);

            let searchIsAuthid: boolean = false;

            // Search a string value
            if (search?.trim()?.length) {
                const regex: RegExp = new RegExp(/^\d{17}$/);
                searchIsAuthid = regex.test(search);
            } else {
                search = this._linkedAccounts.get(userId);
                searchIsAuthid = true;

                // There is no linked account
                if (!search) {

                    const embeded: Discord.EmbedBuilder = new Discord.EmbedBuilder();
                    embeded
                        .setTitle(this._translationService.translate(ITranslateKey.Title_LinkAccount))
                        .setColor(IColor.Success)
                        .setDescription(this._translationService.translate(ITranslateKey.Sentence_LinkNeeded, this._translationService.translate(ITranslateKey.Command_Link)))

                    await interaction.reply({ embeds: [embeded], ephemeral: true });
                    return;
                }
            }


            let map: IMap = null;
            if (mapName) {
                map = await this._mysqlService.getMapByName(mapName);

                if (map == null) {
                    this._sendError(interaction, this._translationService.translate(ITranslateKey.Sentence_MapNotFound));
                    return;
                }
            }

            const player: IPlayer = searchIsAuthid ? await this._mysqlService.getRankByAuthid(search, group, map?.id) : await this._mysqlService.getRankByName(search, group, map?.id);

            if (!player) {
                this._sendError(interaction, this._translationService.translate(ITranslateKey.Sentence_PlayerNotFound));
                return;
            }

            player.metaData = await this.processMetaData(player);

            const embeded: Discord.EmbedBuilder = new Discord.EmbedBuilder();
            const title: string = mapName ? this._translationService.translate(ITranslateKey.Sentence_MapRank, map.name, player.rank) : this._translationService.translate(ITranslateKey.Sentence_Rank, player.rank);

            embeded
                .setAuthor({ name: title })
                .setDescription(this._translationService.translate(ITranslateKey.Sentence_Points, player.points))
                .setTitle(player.metaData?.steamProfile?.name)
                .setURL(`https://steamcommunity.com/profiles/${player.authid}`)
                .setThumbnail(player.metaData.steamProfile?.picture)
                .setFooter({ text: moment(new Date()).locale(process.env.LOCALE ?? "en").format('LLL') })
                .addFields({
                    name: this._translationService.translate(ITranslateKey.StatsTitle_Kills, player.metaData.kill.total),
                    value: this._translationService.translate(ITranslateKey.Stats_Kills, player.metaData.kill.ct, player.metaData.kill.t, player.metaData.headshot, player.metaData.knife),
                    inline: false
                })
                .addFields({
                    name: this._translationService.translate(ITranslateKey.StatsTitle_Assits, player.metaData.killAssist.total),
                    value: this._translationService.translate(ITranslateKey.Stats_Assits, player.metaData.killAssist.ct, player.metaData.killAssist.t),
                    inline: false
                })
                .addFields({
                    name: this._translationService.translate(ITranslateKey.StatsTitle_Deaths, player.metaData.death.total),
                    value: this._translationService.translate(ITranslateKey.Stats_Assits, player.metaData.death.ct, player.metaData.death.t, player.metaData.death.suicide),
                    inline: false
                })
                .addFields({
                    name: this._translationService.translate(ITranslateKey.StatsTitle_Bomb),
                    value: this._translationService.translate(ITranslateKey.Stats_Bomb, player.metaData.bomb.planted, player.metaData.bomb.exploded, player.metaData.bomb.defused),
                    inline: false
                });

            if (player.rank > 0) {
                embeded
                    .setAuthor({ name: this._translationService.translate(ITranslateKey.Sentence_Rank, player.rank) })
                    .setColor(IColor.Success);
            } else {
                embeded
                    .setAuthor({ name: this._translationService.translate(ITranslateKey.Sentence_NoRank) })
                    .setColor(IColor.Warning);
            }

            await interaction.reply({ embeds: [embeded], ephemeral: true });
        } catch (error: any) {
            this._sendError(interaction, this._translationService.translate(ITranslateKey.Sentence_ActionNotPossible));
            this._loggerService.error(error);
        }
    }

    private async _sendTop(interaction: Discord.ChatInputCommandInteraction<Discord.CacheType>): Promise<void> {
        try {
            const group: string = interaction.options.getString('group', true);
            const mapName: string = interaction.options.getString('map', false);

            let map: IMap = null;
            if (mapName) {
                map = await this._mysqlService.getMapByName(mapName);

                if (map == null) {
                    this._sendError(interaction, this._translationService.translate(ITranslateKey.Sentence_MapNotFound));
                    return;
                }
            }

            const players: ITopPlayer[] = await this._mysqlService.getTop(group, map?.id);

            let content: string = "";

            let index = 1;
            for (let player of players) {
                content = content + `\n` + this._translationService.translate(ITranslateKey.Stats_TopPlayer, index, player.name, player.points);
                index++;
            }

            const title: string = mapName ? this._translationService.translate(ITranslateKey.Title_MapTopPlayers, map.name) : this._translationService.translate(ITranslateKey.Title_TopPlayers);

            const embeded: Discord.EmbedBuilder = new Discord.EmbedBuilder();
            embeded
                .setTitle(title)
                .setColor(IColor.Success)
                .setDescription(content)

            await interaction.reply({ embeds: [embeded], ephemeral: true });
        } catch (error: any) {
            this._sendError(interaction, this._translationService.translate(ITranslateKey.Sentence_ActionNotPossible));
            this._loggerService.error(error);
        }
    }

    private async _sendLink(interaction: Discord.ChatInputCommandInteraction<Discord.CacheType>): Promise<void> {
        try {
            const userId: string = interaction.user.id;
            const authid: string = interaction.options.getString('steamid64', true);

            // Discord already linked
            if (this._linkedAccounts.has(userId)) {
                const embeded: Discord.EmbedBuilder = new Discord.EmbedBuilder();
                embeded
                    .setTitle(this._translationService.translate(ITranslateKey.Title_LinkAccount))
                    .setDescription(this._translationService.translate(ITranslateKey.Sentence_DiscordAlreadyLinked))
                    .setColor(IColor.Danger);

                await interaction.reply({ embeds: [embeded], ephemeral: true });

                return;
            }

            // Steam already linked
            const linkedAuthids: string[] = Array.from(this._linkedAccounts.values());
            if (linkedAuthids?.includes(authid)) {
                const embeded: Discord.EmbedBuilder = new Discord.EmbedBuilder();
                embeded
                    .setTitle(this._translationService.translate(ITranslateKey.Title_LinkAccount))
                    .setDescription(this._translationService.translate(ITranslateKey.Sentence_SteamAlreadyLinked))
                    .setColor(IColor.Danger);

                await interaction.reply({ embeds: [embeded], ephemeral: true });
                return;
            }

            // Already a registration request or request expired
            const requestLink: RequestLink = this._requestLinkedAccounts.get(userId);
            const now: moment.Moment = moment();

            if (requestLink && now.isBefore(requestLink?.expire)) {
                const steamProfile: ISteamProfile = await this._steamService.getProfile(requestLink.authid);

                // Matching, register client
                if (steamProfile.name == requestLink.name) {
                    this._requestLinkedAccounts.delete(userId);
                    this._linkedAccounts.set(userId, requestLink.authid);

                    await this._mysqlService.createLinkedAccount(authid, userId);

                    const embeded: Discord.EmbedBuilder = new Discord.EmbedBuilder();
                    embeded
                        .setTitle(this._translationService.translate(ITranslateKey.Title_LinkAccount))
                        .setDescription(this._translationService.translate(ITranslateKey.Sentence_DiscordLinked))
                        .setColor(IColor.Success);

                    await interaction.reply({ embeds: [embeded], ephemeral: true });
                } else {
                    await this._sendRenameNeeded(interaction, requestLink);
                }

                return;
            }

            const milis: number = new Date().getTime();
            const newRequestLink: RequestLink = {
                authid: authid,
                name: "CS2RANK_" + milis,
                expire: moment().add(5, "minutes")
            };

            this._requestLinkedAccounts.set(userId, newRequestLink);
            await this._sendRenameNeeded(interaction, newRequestLink);
        } catch (error: any) {
            this._sendError(interaction, this._translationService.translate(ITranslateKey.Sentence_ActionNotPossible));
            this._loggerService.error(error);
        }
    }

    private async _sendRenameNeeded(interaction: Discord.ChatInputCommandInteraction<Discord.CacheType>, requestLink: RequestLink): Promise<void> {
        const embeded: Discord.EmbedBuilder = new Discord.EmbedBuilder();
        embeded
            .setTitle(this._translationService.translate(ITranslateKey.Title_LinkAccount))
            .setColor(IColor.Info)
            .setDescription(this._translationService.translate(ITranslateKey.Sentence_RenameNeeded, requestLink.name, this._translationService.translate(ITranslateKey.Command_Link)))
            .setFooter({ text: moment(requestLink.expire).locale(process.env.LOCALE ?? "en").format('LL LTS') })

        await interaction.reply({ embeds: [embeded], ephemeral: true });
    }

    private async _sendError(interaction: Discord.ChatInputCommandInteraction<Discord.CacheType>, error: string): Promise<void> {
        const embeded: Discord.EmbedBuilder = new Discord.EmbedBuilder();
        embeded
            .setTitle(this._translationService.translate(ITranslateKey.Title_Error))
            .setColor(IColor.Danger)
            .setDescription(error);
        await interaction.reply({ embeds: [embeded], ephemeral: true });
    }

    private async processMetaData(player: IPlayer): Promise<IPlayerMetaData> {
        const totalKill: number = player.kill_ct + player.kill_t;
        const totalDeath: number = player.death_ct + player.death_t;
        const tototalKillAssit: number = player.killassist_ct + player.killassist_ct;

        const minimumPoints: number = Number(process.env.MINIMUM_POINTS);

        const metaData: IPlayerMetaData = {
            missingRankPoints: minimumPoints - player.points,
            headshot: this.getPercent(player.kill_headshot, totalKill),
            knife: this.getPercent(player.kill_knife, totalKill),
            death: {
                total: totalDeath,
                ct: this.getPercent(player.death_ct, totalDeath),
                t: this.getPercent(player.death_t, totalDeath),
                suicide: this.getPercent(player.death_suicide, totalDeath),
            },
            kill: {
                total: totalKill,
                ct: this.getPercent(player.kill_ct, totalKill),
                t: this.getPercent(player.kill_t, totalKill),
            },
            killAssist: {
                total: tototalKillAssit,
                ct: this.getPercent(player.killassist_ct, tototalKillAssit),
                t: this.getPercent(player.killassist_t, tototalKillAssit),
            },
            bomb: {
                planted: player.bomb_planted,
                exploded: player.bomb_exploded,
                defused: player.bomb_defused
            }
        }

        metaData.steamProfile = await this._steamService.getProfile(player.authid);

        return metaData;
    }

    private getPercent(want: number, total: number): number {
        if (want <= 0 || total <= 0) {
            return 0;
        }

        const percentage: number = (want / total) * 100;
        return parseFloat(percentage.toFixed(0));
    }
}