import { ChannelType, SlashCommandBuilder, SlashCommandChannelOption, SlashCommandIntegerOption, SlashCommandStringOption } from "discord.js";
import { IGroup } from "../interface/IGroup";

export function rankCommand(groups: IGroup[]): SlashCommandBuilder {
    const command: any = new SlashCommandBuilder()
        .setName('rank')
        .setDescription('Get your rank')
        .addStringOption((option: SlashCommandStringOption) =>
            option
                .setName('group')
                .setDescription('Select the group')
                .setRequired(true)
                .addChoices(
                    {
                        name: "FFA 2 - custom label",
                        value: "ffa1"
                    },
                    {
                        name: "Français - custom label",
                        value: "ffa2"
                    }
                ))
        .addStringOption((option: SlashCommandStringOption) =>
            option
                .setName('player')
                .setDescription('Enter authid or name')
                .setRequired(true)
        );

    return command;
}

export function topCommand(groups: IGroup[]): SlashCommandBuilder {
    const command: any = new SlashCommandBuilder()
        .setName('top')
        .setDescription('Get the top')
        .addStringOption((option: SlashCommandStringOption) =>
            option
                .setName('group')
                .setDescription('Select the group')
                .setRequired(true)
                .addChoices(
                    {
                        name: "FFA 2 - custom label",
                        value: "ffa1"
                    },
                    {
                        name: "Français - custom label",
                        value: "ffa2"
                    }
                ))
        .addStringOption((option: SlashCommandStringOption) =>
            option
                .setName('player')
                .setDescription('Enter authid or name')
                .setRequired(true)
        );

    return command;
}