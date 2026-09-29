export function allFids(schema){return Object.keys(schema).map(Number).sort((a,b)=>a-b)}
export function fieldEl(fid){return document.getElementById('f'+fid)}
export function readField(fid){
  const el=fieldEl(fid);
  return el?el.value:undefined;
}
