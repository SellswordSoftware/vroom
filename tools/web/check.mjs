import assert from "node:assert/strict";
import { createServer } from "node:http";
import { readFile, mkdir } from "node:fs/promises";
import { resolve, extname } from "node:path";
import { fileURLToPath } from "node:url";
import { chromium } from "playwright";

const game = fileURLToPath(new URL("../../", import.meta.url));
const site = resolve(game, "site");
const out = resolve(game, "out/web");
await mkdir(out, {recursive:true});
const prefix = "/vroom-preview/";
const types = {".html":"text/html",".js":"text/javascript",".css":"text/css",".wasm":"application/wasm",".svg":"image/svg+xml"};
const server = createServer(async (req,res) => {
  try {
    const url = new URL(req.url, "http://localhost");
    if (!url.pathname.startsWith(prefix)) {res.writeHead(404);res.end();return;}
    const name = decodeURIComponent(url.pathname.slice(prefix.length)) || "index.html";
    const path = resolve(site,name);
    if (!path.startsWith(site + "/")) throw Error("invalid path");
    const data = await readFile(path);
    res.writeHead(200,{"Content-Type":types[extname(path)] || "application/octet-stream"});res.end(data);
  } catch {res.writeHead(404);res.end();}
});
await new Promise(r => server.listen(0,"127.0.0.1",r));
const base = "http://127.0.0.1:" + server.address().port + prefix;
const browser = await chromium.launch({headless:true,args:["--enable-unsafe-swiftshader"]});
const errors = [];
const requests = [];
const setup = page => {
  page.on("pageerror",e => errors.push(String(e)));
  page.on("request",r => requests.push(r.url()));
  page.on("response",r => {if(r.status()>=400) errors.push(r.status()+" "+r.url());});
};
const ready = async page => {
  await page.waitForFunction(() => document.body.dataset.state==="ready", null, {timeout:20000});
  await page.waitForTimeout(250);
};
const tap = async (page,key) => {
  await page.keyboard.down(key);await page.waitForTimeout(100);
  await page.keyboard.up(key);await page.waitForTimeout(100);
};

async function checkFrame(page) {
  // Inspect the screenshot itself; successful CPU/save activity alone does not
  // prove that a browser's graphics backend rendered the LCD correctly.
  const png = await page.locator("#canvas").screenshot();
  const colors = await page.evaluate(async url => {
    const img = new Image();img.src=url;await img.decode();
    const canvas=document.createElement("canvas");canvas.width=img.width;canvas.height=img.height;
    const ctx=canvas.getContext("2d");ctx.drawImage(img,0,0);
    const bytes=ctx.getImageData(0,0,img.width,img.height).data;
    let blue=0,gold=0,black=0;
    for(let i=0;i<bytes.length;i+=4){
      const r=bytes[i],g=bytes[i+1],b=bytes[i+2];
      if(b>70&&b>r*1.5&&b>g*1.2)blue++;
      if(r>180&&g>140&&b<120)gold++;
      if(r<30&&g<30&&b<30)black++;
    }
    const n=img.width*img.height;return {blue:blue/n,gold:gold/n,black:black/n};
  },"data:image/png;base64,"+png.toString("base64"));
  assert(colors.blue>.01 && colors.gold>.0002 && colors.black>.02,
    "LCD must render the actual blue/gold Vroom palette, not blank pixels: "+JSON.stringify(colors));
}
const saved = () => {
  const bytes = Module.FS.readFile("/chgame/saves/vroom.sav");
  const view = new DataView(bytes.buffer, bytes.byteOffset, bytes.byteLength);
  const records=[];
  for(let i=0;i+120<bytes.length;i++){
    if(bytes[i]===86&&bytes[i+1]===82&&bytes[i+2]===77&&bytes[i+3]===49&&bytes[i+4]===2)
      records.push({seq:view.getUint16(i+6,true),cash:view.getUint32(i+8,true),wins:bytes[i+112]});
  }
  records.sort((a,b)=>b.seq-a.seq);
  return {bytes:Array.from(bytes),record:records[0]};
};
try {
  const context = await browser.newContext({viewport:{width:1440,height:1000}});
  const page = await context.newPage();setup(page);
  // An external ROM query must be ignored; only the committed Vroom file loads.
  await page.goto(base+"?rom=https://example.invalid/another-game.bin");
  await ready(page);
  assert.equal(await page.title(),"Vroom — Shift King");
  assert.equal(await page.locator('input[type="file"],select,iframe').count(),0);
  assert.equal(await page.locator("[data-key]:disabled").count(),0);
  assert.equal(await page.evaluate(()=>Module.FS.readdir("/roms").filter(x=>x!=="."&&x!=="..").join(",")),"vroom.bin");
  assert(!requests.some(x=>new URL(x).hostname==="example.invalid"));
  assert(!requests.some(x=>/games.json|sdcard.zip/.test(x)));
  await checkFrame(page);
  await page.screenshot({path:out+"/desktop.png",fullPage:true});
  console.log("PASS automatic Vroom-only boot at a project subpath; no picker or external ROM");

  await tap(page,"x");await tap(page,"x");
  await page.waitForTimeout(7000);await tap(page,"x");
  await page.waitForTimeout(3000);await tap(page,"x");
  await page.screenshot({path:out+"/race.png"});
  await page.waitForFunction(()=>Module.FS.analyzePath("/chgame/saves/vroom.sav").exists,null,{timeout:45000});
  await page.waitForTimeout(1200);
  const before = await page.evaluate(saved);
  assert(before.record && before.record.cash>0,"a real completed race must award saved cash");
  await checkFrame(page);
  await page.screenshot({path:out+"/result.png"});
  // The emulator performs its own IndexedDB sync; reload tests the persisted copy.
  await page.reload();await ready(page);
  const after = await page.evaluate(saved);
  assert.deepEqual(after.bytes,before.bytes);
  console.log("PASS keyboard race completion and IndexedDB save restoration; cash "+after.record.cash);

  // Holding START must never strand this one-game site in a console loader.
  await page.keyboard.down("Enter");await page.waitForTimeout(3600);
  await page.keyboard.up("Enter");await page.waitForTimeout(1500);
  await checkFrame(page);
  const home = await page.locator("#canvas").screenshot();
  await tap(page,"x");await page.waitForTimeout(150);
  const navigated = await page.locator("#canvas").screenshot();
  assert(!home.equals(navigated),"game must remain interactive after held START");
  console.log("PASS held START stays in playable Vroom");

  // Pointer inputs release on blur, even when two controls are held.
  const a = await page.locator('[data-key="KeyX"]').boundingBox();
  await page.mouse.move(a.x+a.width/2,a.y+a.height/2);
  await page.mouse.down();
  assert.equal(await page.locator(".pressed").count(),1);
  await page.evaluate(()=>window.dispatchEvent(new Event("blur")));
  assert.equal(await page.locator(".pressed").count(),0);
  await page.mouse.up();
  await page.keyboard.down("x");
  assert.equal(await page.locator('[data-key="KeyX"].pressed').count(),1);
  await page.evaluate(()=>document.querySelector('[data-key="KeyX"]').click());
  await page.waitForTimeout(200);
  assert.equal(await page.locator('[data-key="KeyX"].pressed').count(),1,"a virtual tap must not release a physical hold");
  await page.keyboard.up("x");
  assert.equal(await page.locator(".pressed").count(),0);
  console.log("PASS blur release and overlapping physical/virtual input");

  await page.getByRole("button",{name:"FULLSCREEN"}).click();
  await page.waitForFunction(()=>!!document.fullscreenElement);
  assert(await page.locator(".console-controls").isVisible());
  await page.evaluate(()=>document.exitFullscreen());
  console.log("PASS fullscreen");
  await context.close();

  const mobile = await browser.newContext({viewport:{width:390,height:844},deviceScaleFactor:2,isMobile:true,hasTouch:true});
  const phone = await mobile.newPage();setup(phone);await phone.goto(base);await ready(phone);
  assert(await phone.evaluate(()=>document.documentElement.scrollWidth<=innerWidth),"mobile layout overflows");
  const sizes = await phone.evaluate(()=>({w:Module.canvas.width,h:Module.canvas.height,dpr:devicePixelRatio}));
  console.log("Mobile canvas geometry:",JSON.stringify(sizes));
  await phone.touchscreen.tap(...await phone.locator('[data-key="KeyX"]').evaluate(el=>{
    const r=el.getBoundingClientRect();return [r.x+r.width/2,r.y+r.height/2];
  }));
  await phone.waitForTimeout(300);
  await checkFrame(phone);
  await phone.screenshot({path:out+"/mobile.png",fullPage:true});
  assert.equal(await phone.locator(".pressed").count(),0);
  // Browser reload preserves this origin's save filename across ROM URL versions.
  await mobile.close();

  const failure = await browser.newContext();
  await failure.route("**/vroom.bin?*",route=>route.fulfill({status:404,body:"not found"}));
  const errorPage=await failure.newPage();
  await errorPage.goto(base);
  await errorPage.waitForFunction(()=>document.body.dataset.state==="error");
  assert.equal(await errorPage.locator("#error").isVisible(),true);
  assert.equal(await errorPage.getByRole("button",{name:"Try again"}).isVisible(),true);
  await failure.close();
  console.log("PASS mobile touch layout and actionable missing-ROM error");

  assert.deepEqual(errors,[]);
  console.log("ALL WEB CHECKS PASSED; screenshots: "+out);
} finally {
  await browser.close();
  await new Promise(r=>server.close(r));
}
