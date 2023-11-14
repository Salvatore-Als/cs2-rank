import * as Discord from 'discord.js';
import { Partials, Routes, REST, SlashCommandBuilder, SlashCommandStringOption } from "discord.js";
import _ from 'lodash';
import { Inject, Singleton } from 'typescript-ioc';
import LoggerService from '../services/loggerService';
import MysqlService from '../services/mysqlService';
import { IGroup } from '../interface/IGroup';
import TranslationService from '../services/translationService';
import { ITranslateKey } from '../interface/ITranslate';


@Singleton
export default class DiscordProvider {
    @Inject
    private _loggerService: LoggerService;

    @Inject
    private _mysqlService: MysqlService;

    @Inject
    private _translationService: TranslationService;

    private _rest: Discord.REST;

    private _client: Discord.Client = new Discord.Client({
        partials: [
            Partials.Message,
            Partials.Channel,
        ],
        intents: []
    });

    get client(): Discord.Client {
        return this._client;
    }

    constructor() {
        this._rest = new REST({ version: '10' }).setToken(process.env.DISCORD_TOKEN as string);

        process.on('SIGINT', () => {
            process.exit()
        });

        process.on('exit', () => {
            this._client.destroy();
        });
    }

    public async run(): Promise<void> {
        return new Promise(async (resolve, reject) => {
            const groups: IGroup[] = await this._mysqlService.getGroups();
            const groupMapping = groups.map((group: IGroup) => {
                return { name: group.custom_name ? group.custom_name : group.reference, value: group.reference }
            });

            const start = new Date().getTime();

            this._client.login(process.env.DISCORD_TOKEN as string);

            this._client.on('ready', async (client: Discord.Client<true>) => {
                await this._rest.put(
                    Routes.applicationCommands(client.user.id), {
                    body: [this.buildRankCommand(groupMapping).toJSON(), this.buildTopCommand(groupMapping).toJSON(), this.buildLinkCommand(groupMapping).toJSON()]
                });

                this._loggerService.info("[Discord Provider] Running " + (new Date().getTime() - start) / 1000);
                return resolve();
            });

            this._client.on('error', (error) => {
                return reject("[Discord Provider] Connect error " + error);
            });
        });
    }

    private buildRankCommand(groups: { name: string, value: string }[]): SlashCommandBuilder {
        const command: any = new SlashCommandBuilder()
            .setName(this._translationService.translate(ITranslateKey.Command_Rank))
            .setDescription(this._translationService.translate(ITranslateKey.Command_DescriptionRank))
            .addStringOption((option: SlashCommandStringOption) =>
                option
                    .setName('group')
                    .setDescription(this._translationService.translate(ITranslateKey.CommandArg_DescriptionGroup))
                    .setRequired(true)
                    .addChoices(...groups))
            .addStringOption((option: SlashCommandStringOption) =>
                option
                    .setName('player')
                    .setDescription(this._translationService.translate(ITranslateKey.CommandArg_DescriptionPlayer))
                    .setRequired(false)
            );

        return command;
    }

    private buildTopCommand(groups: { name: string, value: string }[]): SlashCommandBuilder {
        const command: any = new SlashCommandBuilder()
            .setName(this._translationService.translate(ITranslateKey.Command_Top))
            .setDescription(this._translationService.translate(ITranslateKey.Command_DescriptionTop))
            .addStringOption((option: SlashCommandStringOption) =>
                option
                    .setName('group')
                    .setDescription(this._translationService.translate(ITranslateKey.CommandArg_DescriptionGroup))
                    .setRequired(true)
                    .addChoices(...groups)
            );

        return command;
    }

    private buildLinkCommand(groups: { name: string, value: string }[]): SlashCommandBuilder {
        const command: any = new SlashCommandBuilder()
            .setName(this._translationService.translate(ITranslateKey.Command_Link))
            .setDescription(this._translationService.translate(ITranslateKey.Command_Link))
            .addStringOption((option: SlashCommandStringOption) =>
                option
                    .setName('steamid64')
                    .setDescription(this._translationService.translate(ITranslateKey.CommandArg_DescriptionSteamid))
                    .setRequired(true)
            );

        return command;
    }
}