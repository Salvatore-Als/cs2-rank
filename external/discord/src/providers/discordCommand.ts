import { ChannelType, SlashCommandBuilder, SlashCommandChannelOption, SlashCommandIntegerOption, SlashCommandStringOption } from "discord.js";
import { IGroup } from "../interface/IGroup";

export function rankCommand(groups: IGroup[]): SlashCommandBuilder {
    const mapping = groups.map((group: IGroup) => {
        return { name: group.custom_name ? group.custom_name : group.reference, value: group.reference }
    });

    const command: any = new SlashCommandBuilder()
        .setName('rank')
        .setDescription('Get your rank')
        .addStringOption((option: SlashCommandStringOption) =>
            option
                .setName('group')
                .setDescription('Select the group')
                .setRequired(true)
                .addChoices(...mapping))
        .addStringOption((option: SlashCommandStringOption) =>
            option
                .setName('player')
                .setDescription('Enter authid or name')
                .setRequired(true)
        );

    return command;
}

export function topCommand(groups: IGroup[]): SlashCommandBuilder {
    const mapping = groups.map((group: IGroup) => {
        return { name: group.custom_name ? group.custom_name : group.reference, value: group.reference }
    });

    const command: any = new SlashCommandBuilder()
        .setName('top')
        .setDescription('Get the top')
        .addStringOption((option: SlashCommandStringOption) =>
            option
                .setName('group')
                .setDescription('Select the group')
                .setRequired(true)
                .addChoices(...mapping)
        );

    return command;
}