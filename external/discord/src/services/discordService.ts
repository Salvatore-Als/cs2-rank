import * as Discord from 'discord.js';
import { Inject, Singleton } from 'typescript-ioc';
import { EmptyString, IColor } from '../utils/const';
import _ from 'lodash';
import LoggerService from './loggerService';
import DiscordProvider from '../providers/discordProvider';
import moment from 'moment';

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

    private _client: Discord.Client = null;

    // TODO: user action cache to prevent flood

    constructor() {

    }

    public async run(): Promise<void> {
        this._client = this._discordProvider.client;
        this._subscribe();

        this._loggerService.info("[Discord Service] Running ");
    }

    private _subscribe(): void {
        this._client.on('interactionCreate', async (interaction: Discord.Interaction<Discord.CacheType>) => {
            try {
                if (interaction.isChatInputCommand()) {
                    switch (interaction.commandName) {
                        case 'rank':
                            await this._getRank(interaction);
                            break;
                        case 'top':
                            await this._getTop(interaction);
                            break;
                    }

                    return;
                }
            } catch (error: any) {
                this._loggerService.error(`[Discord Provider] ${error.stack} `);
            }
        });
    }

    private async _getRank(interaction: Discord.ChatInputCommandInteraction<Discord.CacheType>): Promise<void> {
        const userId: string = interaction.user.id;
        const player: string = interaction.options.getString('player', true);
        const group: string = interaction.options.getString('group', true);

        const embeded: Discord.EmbedBuilder = new Discord.EmbedBuilder();

        embeded.setAuthor({ name: "KRIAX" })
            .setTitle('You have X points')
            .setDescription(`You are ranked XXX`)
            .setThumbnail("https://avatars.cloudflare.steamstatic.com/a6f6a9c1958fd89781c4ddd94e79ce96bd231275_full.jpg")
            .setFooter({ text: moment(new Date()).locale(process.env.LOCALE ?? "en").format('LLL') });

        embeded.addFields({ name: "Kill: X", value: `%ct %t \n %headshot \n %knife`, inline: true });
        embeded.addFields({ name: "Assist: X", value: `%ct %t`, inline: true });
        embeded.addFields(embededGhostField);
        embeded.addFields({ name: "Death: x", value: `%suicide`, inline: true });
        embeded.addFields({ name: "Bomb: X", value: `%planted \n %defused \n %exploded`, inline: true });

        await interaction.reply({ embeds: [embeded], ephemeral: true });
    }

    private async _getTop(interaction: Discord.ChatInputCommandInteraction<Discord.CacheType>): Promise<void> {
        const userId: string = interaction.user.id;
        const player: string = interaction.options.getString('player', true);
        const group: string = interaction.options.getString('group', true);

        this._loggerService.debug(group);
        this._loggerService.debug(player);

        let embeded: Discord.EmbedBuilder = null
        let content: string = "";

        const embeds: Discord.EmbedBuilder[] = []
        embeded = new Discord.EmbedBuilder();

        embeded
            .setTitle("EMBED TITLE").setColor(IColor.Success)
            .setDescription("EMBED CONTENT");

        embeds.push(embeded);

        await interaction.reply({ embeds: embeds, ephemeral: true });
    }
}