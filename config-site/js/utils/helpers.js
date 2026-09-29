export const $ = s => document.querySelector(s);
export const $$ = s => Array.from(document.querySelectorAll(s));
export const sleep = ms => new Promise(r=>setTimeout(r,ms));
