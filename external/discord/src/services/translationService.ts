import { Inject, Singleton } from "typescript-ioc";
import fs from 'fs';
import path from 'path';
import LoggerService from "./loggerService";
import { ITranslateKey } from "../interface/ITranslate";

@Singleton
export default class TranslateService {
    @Inject
    private _loggerService: LoggerService

    private _translations: Map<string, string> = new Map();
    private _defaultTranslations: Map<string, string> = new Map();

    constructor() {

    }

    public run(): void {
        if (process.env.LOCALE) {
            const file = fs.readFileSync(path.join(__dirname, `../translations/${process.env.LOCALE}.json`), 'utf8');
            this._translations = JSON.parse(JSON.stringify(file));
        }

        const defaultFile = fs.readFileSync(path.join(__dirname, `../translations/default.json`), 'utf8');
        this._defaultTranslations = JSON.parse(JSON.stringify(defaultFile));

        this._loggerService.info('[Translate Service] Running');
    }

    public translate(key: ITranslateKey, ...vars: any): string {
        let translate = this._translations.get(key) ?? this._defaultTranslations.get(key) ?? `MISSING_TRANSLATE_${key}`;

        let count = -1;
        return translate.replace(/#VAR#/g, () => {
            count++;
            return vars[count] !== null ? vars[count] : null;
        });
    }
}