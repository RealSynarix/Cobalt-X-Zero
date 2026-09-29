const EXAMPLES={
hello:`void pio() {
  keyboard.write("Hello from Cobalt-X!");
}`,
copy:`void pio() {
  keyboard.hotkey(KEY_LEFT_CTRL, KEY_C);
}`,
browser:`void pio() {
  keyboard.hotkey(KEY_LEFT_CTRL, KEY_L);
  delay(40);
  keyboard.write("https://example.com");
  keyboard.tap(KEY_ENTER);
}`,
clicks:`void pio() {
  repeat(3) {
    mouse.click(MOUSE_LEFT);
    delay(30);
  }
}`,
movement:`void pio() {
  mouse.move(20, -10);
  delay(20);
  scroll(2);
}`,
custom:`void pio() {

}`
};

const KEY={KEY_LEFT_CTRL:224,KEY_LEFT_SHIFT:225,KEY_LEFT_ALT:226,KEY_LEFT_GUI:227,KEY_RIGHT_CTRL:228,KEY_RIGHT_SHIFT:229,KEY_RIGHT_ALT:230,KEY_RIGHT_GUI:231,KEY_ENTER:40,KEY_ESC:41,KEY_BACKSPACE:42,KEY_TAB:43,KEY_SPACE:44,KEY_MINUS:45,KEY_EQUAL:46,KEY_LEFT_BRACKET:47,KEY_RIGHT_BRACKET:48,KEY_BACKSLASH:49,KEY_SEMICOLON:51,KEY_APOSTROPHE:52,KEY_GRAVE:53,KEY_COMMA:54,KEY_PERIOD:55,KEY_SLASH:56,KEY_CAPS_LOCK:57,KEY_F1:58,KEY_F2:59,KEY_F3:60,KEY_F4:61,KEY_F5:62,KEY_F6:63,KEY_F7:64,KEY_F8:65,KEY_F9:66,KEY_F10:67,KEY_F11:68,KEY_F12:69,KEY_DELETE:76,KEY_HOME:74,KEY_END:77,KEY_PAGE_UP:75,KEY_PAGE_DOWN:78,KEY_RIGHT:79,KEY_LEFT:80,KEY_DOWN:81,KEY_UP:82};
for(let i=0;i<26;i++)KEY['KEY_'+String.fromCharCode(65+i)]=4+i;
for(let i=0;i<10;i++)KEY['KEY_'+i]=(i===0?39:29+i);
const MOUSE={MOUSE_LEFT:0,MOUSE_RIGHT:1,MOUSE_MIDDLE:2,MOUSE_BACK:3,MOUSE_FORWARD:4,MOUSE_5:5,MOUSE_6:6,MOUSE_7:7};
const MEDIA={MEDIA_PLAY_PAUSE:0,MEDIA_NEXT:1,MEDIA_PREVIOUS:2,MEDIA_STOP:3,MEDIA_MUTE:4,MEDIA_VOLUME_UP:5,MEDIA_VOLUME_DOWN:6};

function valueOf(s,table){
  s=s.trim();
  if(Object.prototype.hasOwnProperty.call(table,s))return table[s];
  if(/^[-+]?\d+$/.test(s))return Number(s);
  throw Error(`Unknown value ${s}`);
}
function splitArgs(s){
  const out=[];let q=null,start=0,depth=0;
  for(let i=0;i<s.length;i++){
    const c=s[i];
    if(q){if(c===q&&s[i-1]!=='\\')q=null;continue}
    if(c==='"'||c==="'"){q=c;continue}
    if(c==='('||c==='{')depth++;
    if(c===')'||c==='}')depth--;
    if(c===','&&depth===0){out.push(s.slice(start,i).trim());start=i+1}
  }
  out.push(s.slice(start).trim());
  return out.filter(Boolean);
}
function parseString(s){
  const m=s.match(/^"([\s\S]*)"$/);
  if(!m)throw Error(`Expected quoted string: ${s}`);
  return JSON.parse(s);
}
function expandRepeat(src){
  let out='',i=0;
  while(i<src.length){
    const m=src.slice(i).match(/repeat\s*\(\s*(\d+)\s*\)\s*\{/);
    if(!m){out+=src[i++];continue}
    const start=i+m.index+m[0].length;let depth=1,j=start,q=null;
    for(;j<src.length;j++){
      const c=src[j];
      if(q){if(c===q&&src[j-1]!=='\\')q=null;continue}
      if(c==='"'||c==="'"){q=c;continue}
      if(c==='{')depth++;
      if(c==='}')depth--;
      if(depth===0)break;
    }
    if(depth!==0)throw Error('Unclosed repeat block');
    const body=src.slice(start,j);
    const n=Number(m[1]);
    if(n>64)throw Error('repeat() is limited to 64 iterations');
    out+=Array(n).fill(body).join('\n');
    i=j+1;
  }
  return out;
}
function emitOp(out,op,...bytes){out.push(op,...bytes.map(x=>x&255))}
function emitU16(out,v){emitOp(out,v&255,(v>>8)&255)}
function compileLine(line,out){
  line=line.trim().replace(/;$/,'').trim();
  if(!line||line==='void pio() {'||line==='}')return;
  let m;
  if((m=line.match(/^delay\s*\(\s*(\d+)\s*\)$/))){let v=Number(m[1]);if(v>65535)throw Error('delay max is 65535 ms');emitOp(out,1,v,v>>8);return}
  if((m=line.match(/^mouse\.(press|release|click)\s*\(\s*([^,)]+)\s*(?:,\s*(\d+)\s*)?\)$/))){
    const fn=m[1],b=valueOf(m[2],MOUSE),count=Number(m[3]||1);
    if(b<0||b>7||count<1||count>32)throw Error('Invalid mouse button or click count');
    if(fn==='press')emitOp(out,2,b);
    else if(fn==='release')emitOp(out,3,b);
    else for(let i=0;i<count;i++){emitOp(out,2,b);emitOp(out,1,5,0);emitOp(out,3,b);if(i+1<count)emitOp(out,1,5,0)}
    return;
  }
  if((m=line.match(/^mouse\.move\s*\(\s*(-?\d+)\s*,\s*(-?\d+)\s*\)$/))){
    const x=Number(m[1]),y=Number(m[2]);if(x<-127||x>127||y<-127||y>127)throw Error('mouse.move values must be -127..127');emitOp(out,10,x,y);return;
  }
  if((m=line.match(/^scroll\s*\(\s*(-?\d+)\s*\)$/))){
    const v=Number(m[1]);if(v<-127||v>127)throw Error('scroll value must be -127..127');emitOp(out,9,v);return;
  }
  if((m=line.match(/^keyboard\.(press|release|tap)\s*\(\s*([^)]+)\)$/))){
    const fn=m[1],k=valueOf(m[2],KEY);if(k<0||k>255)throw Error('Invalid HID key');
    emitOp(out,fn==='press'?4:fn==='release'?5:6,k);return;
  }
  if((m=line.match(/^keyboard\.write\s*\(\s*(\"[\s\S]*\")\s*\)$/))){
    const s=parseString(m[1]);const bytes=new TextEncoder().encode(s);if(bytes.length>255)throw Error('keyboard.write max is 255 bytes');
    emitOp(out,7,bytes.length,...bytes);return;
  }
  if((m=line.match(/^keyboard\.hotkey\s*\(\s*([^)]+)\)$/))){
    const a=splitArgs(m[1]);if(a.length>6)throw Error('hotkey max is 6 keys');
    const keys=a.map(x=>valueOf(x,KEY));emitOp(out,11,a.length,...keys);return;
  }
  if((m=line.match(/^media\.tap\s*\(\s*([^)]+)\)$/))){
    const v=valueOf(m[1],MEDIA);if(v<0||v>12)throw Error('Invalid media key');emitOp(out,8,v);return;
  }
  if(line==='keyboard.releaseAll()'){emitOp(out,12);return}
  throw Error(`Unsupported PIO statement: ${line}`);
}
export function compilePIO(source){
  let clean='',quote=null;
  for(let i=0;i<source.length;i++){
    const c=source[i],n=source[i+1];
    if(quote){clean+=c;if(c===quote&&source[i-1]!=='\\')quote=null;continue}
    if(c==='"'||c==="'"){quote=c;clean+=c;continue}
    if(c==='/'&&n==='/'){while(i<source.length&&source[i]!=="\\n")i++;clean+='\\n';continue}
    clean+=c;
  }
  const expanded=expandRepeat(clean);
  const body=expanded.replace(/^\s*void\s+pio\s*\(\s*\)\s*\{/, '').replace(/\}\s*$/, '');
  const lines=body.split(/\r?\n/);
  const out=[];
  for(const line of lines)compileLine(line,out);
  out.push(0);
  if(out.length>444)throw Error(`Program is ${out.length} bytes; maximum is 444 bytes`);
  const all=[0xC7,1,out.length&255,(out.length>>8)&255,...out];
  while(all.length<448)all.push(0);
  return new Uint8Array(all);
}
export function exampleSource(name){return EXAMPLES[name]||EXAMPLES.custom}
export function hexChunks(bytes){
  const chunks=[];
  for(let i=0;i<448;i+=56)chunks.push(Array.from(bytes.slice(i,i+56)).map(x=>x.toString(16).padStart(2,'0')).join(''));
  return chunks;
}
export function chunksHexToBytes(chunks){
  const out=new Uint8Array(448);
  let at=0;
  for(const h of chunks){
    if(!h)continue;
    for(let i=0;i<h.length&&at<448;i+=2)out[at++]=parseInt(h.slice(i,i+2),16)||0;
  }
  return out;
}
export {EXAMPLES};
