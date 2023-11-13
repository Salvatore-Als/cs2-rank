import * as Discord from 'discord.js';
import { Partials, Routes, REST } from "discord.js";
import _ from 'lodash';
import { Inject, Singleton } from 'typescript-ioc';
import LoggerService from '../services/loggerService';
import { rankCommand, topCommand } from './discordCommand';


@Singleton
export default class DiscordProvider {
    @Inject
    private _loggerService: LoggerService;

    private _intents: Discord.IntentsBitField = new Discord.IntentsBitField([
        Discord.IntentsBitField.Flags.GuildMembers,
        Discord.IntentsBitField.Flags.GuildMessages
    ]);

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
        return new Promise((resolve, reject) => {
            const start = new Date().getTime();

            this._client.login(process.env.DISCORD_TOKEN as string);

            this._client.on('ready', async (client: Discord.Client<true>) => {
                await this._rest.put(
                    Routes.applicationCommands(client.user.id), {
                    body: [rankCommand([]).toJSON(), topCommand([]).toJSON()]
                });

                this._loggerService.info("[Discord Provider] Running " + (new Date().getTime() - start) / 1000);
                return resolve();
            });

            this._client.on('error', (error) => {
                return reject("[Discord Provider] Connect error " + error);
            });
        });
    }
}