import express from 'express';

export class CError extends Error {

    public message: string;
    public status: number;

    constructor(message: any, code: any) {
        super();
        this.message = message;
        this.status = code;
    }
}