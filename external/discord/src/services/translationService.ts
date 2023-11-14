import { Inject, Singleton } from "typescript-ioc";
import fs from 'fs';
import path from 'path';
import LoggerService from "./loggerService";
import { ITranslateKey } from "../interface/ITranslate";

@Singleton
export default class TranslationService {
    @Inject
    private _loggerService: LoggerService

    private _translations: any = {};
    private _defaultTranslations: any = {};

    constructor() {

    }

    public run(): void {
        if (process.env.LOCALE) {
            const file = fs.readFileSync(path.join(__dirname, `../translations/${process.env.LOCALE}.json`), 'utf8');
            this._translations = JSON.parse(file);
        }

        const defaultFile = fs.readFileSync(path.join(__dirname, `../translations/default.json`), 'utf8');
        this._defaultTranslations = JSON.parse(defaultFile);

        this._loggerService.info('[Translate Service] Running');
    }

    public translate(key: ITranslateKey, ...vars: any): string {
        const translate = this._translations[key] ?? this._defaultTranslations[key] ?? `404_${key}`;

        let count = -1;
        return translate.replace(/#VAR#/g, () => {
            count++;
            return vars[count] !== null ? vars[count] : null;
        });
    }
}