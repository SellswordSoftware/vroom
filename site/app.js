/* Vroom-only shell for the unmodified CHGame WebAssembly emulator. */
(() => {
  "use strict";
  const canvas = document.getElementById("canvas");
  const status = document.getElementById("status");
  const saveStatus = document.getElementById("save-status");
  const controls = [...document.querySelectorAll("[data-key]")];
  const config = window.VROOM_CONFIG;
  let ready = false;
  const held = new Map();
  const pointers = new Map();
  let pointerSequence = 0;
  const keyInfo = {
    ArrowUp: ["ArrowUp", 38], ArrowDown: ["ArrowDown", 40],
    ArrowLeft: ["ArrowLeft", 37], ArrowRight: ["ArrowRight", 39],
    KeyX: ["x", 88], KeyZ: ["z", 90], Enter: ["Enter", 13], ShiftRight: ["Shift", 16]
  };
  const aliases = {Space:"KeyX",KeyK:"KeyX",KeyJ:"KeyZ",KeyW:"ArrowUp",
    KeyS:"ArrowDown",KeyA:"ArrowLeft",KeyD:"ArrowRight",Backspace:"ShiftRight"};
  function sound() { window.Module?.SDL3?.audioContext?.resume().catch(() => {}); }
  function fail(error) {
    ready = false;
    controls.forEach(button => { button.disabled = true; });
    status.textContent = "Engine stopped";
    document.getElementById("error-text").textContent = "Vroom could not start. Check your connection and try again. " + error.message;
    document.getElementById("error").hidden = false;
    document.body.dataset.state = "error";
  }
  function send(code, down) {
    const [key, keyCode] = keyInfo[code];
    const event = new KeyboardEvent(down ? "keydown" : "keyup", {
      key, code, keyCode, which:keyCode, location:code==="ShiftRight"?2:0,
      bubbles:true, cancelable:true
    });
    Object.defineProperty(event, "vroomInput", {value:true});
    canvas.dispatchEvent(event);
    controls.filter(button => button.dataset.key === code)
      .forEach(button => button.classList.toggle("pressed", down));
  }
  function input(code, owner, down) {
    if (!ready) return;
    const sources = held.get(code) || new Set();
    const before = sources.size;
    if (down) sources.add(owner); else sources.delete(owner);
    held.set(code, sources);
    if (!!before !== !!sources.size) send(code, !!sources.size);
    if (down) sound();
  }
  function releaseAll() {
    for (const [code, owners] of held) { if (owners.size) send(code, false); }
    held.clear(); pointers.clear();
  }
  for (const type of ["keydown", "keyup"]) {
    window.addEventListener(type, event => {
      if (event.vroomInput) return;
      // Keep browser shortcuts and native button activation, but never forward
      // emulator menus, file dialogs, fast-forward or bootloader shortcuts.
      if (event.ctrlKey || event.metaKey || event.altKey ||
          (event.target.closest?.("button,a,summary") && ["Enter","Space"].includes(event.code))) {
        event.stopImmediatePropagation(); return;
      }
      const code = aliases[event.code] || event.code;
      if (keyInfo[code]) {
        event.preventDefault(); event.stopImmediatePropagation();
        input(code, "keyboard:" + event.code, type === "keydown");
      } else {
        event.stopImmediatePropagation();
      }
    }, true);
  }
  for (const button of controls) {
    button.addEventListener("pointerdown", event => {
      event.preventDefault();
      if (!ready) return;
      button.setPointerCapture(event.pointerId);
      const owner = "pointer:" + (++pointerSequence);
      pointers.set(event.pointerId, {code:button.dataset.key, owner, at:performance.now()});
      input(button.dataset.key, owner, true);
      canvas.focus({preventScroll:true});
    });
    const release = event => {
      const pointer = pointers.get(event.pointerId);
      if (!pointer) return;
      pointers.delete(event.pointerId);
      // The emulated game polls at 60 Hz: very short taps must span a frame.
      const delay = Math.max(0, 50 - (performance.now() - pointer.at));
      setTimeout(() => input(pointer.code, pointer.owner, false), delay);
    };
    for (const type of ["pointerup","pointercancel","lostpointercapture"])
      button.addEventListener(type, release);
    button.addEventListener("contextmenu", event => event.preventDefault());
    // Screen-reader/native keyboard activation still works without a pointer.
    button.addEventListener("click", event => {
      if (event.detail !== 0) return;
      const owner = "click:" + button.dataset.key;
      input(button.dataset.key, owner, true);
      setTimeout(() => input(button.dataset.key, owner, false), 120);
    });
  }
  window.addEventListener("blur", releaseAll);
  document.addEventListener("visibilitychange", () => { if (document.hidden) releaseAll(); });
  for (const type of ["dragover","drop"])
    window.addEventListener(type, event => { event.preventDefault(); event.stopImmediatePropagation(); }, true);
  canvas.addEventListener("contextmenu", event => event.preventDefault());
  canvas.addEventListener("pointerdown", sound);
  document.getElementById("fullscreen").addEventListener("click", async () => {
    try {
      if (document.fullscreenElement) await document.exitFullscreen();
      else await document.querySelector(".cockpit").requestFullscreen();
      canvas.focus({preventScroll:true}); sound();
    } catch { status.textContent = "Fullscreen is unavailable in this browser"; }
  });

  document.addEventListener("fullscreenchange", () => {
    document.getElementById("fullscreen").textContent =
      document.fullscreenElement ? "EXIT FULLSCREEN ↙" : "FULLSCREEN ↗";
  });

  // This ROM path is stable across releases: the emulator keys saves by filename.
  // All URLs are relative so project Pages sites work at /repository-name/.
  const rom = fetch("vroom.bin?v=" + config.romHash).then(response => {
    if (!response.ok) throw new Error("Game download failed (" + response.status + ")");
    return response.arrayBuffer();
  });
  rom.catch(() => {}); // handled below even if it fails before runtime startup
  window.Module = {
    arguments:["--no-bootloader","--no-sd","--integer-scale"],
    canvas,
    locateFile: path => new URL("vendor/" + path + "?v=" + config.engineHash, document.baseURI).href,
    print: () => {},
    printErr: text => console.debug("[CHGame]", text),
    onAbort: reason => fail(new Error(String(reason))),
    postRun:[() => {
      const FS = window.Module.FS;
      FS.syncfs(true, async error => {
        if (error) saveStatus.textContent = "Storage unavailable. Progress lasts for this session.";
        try {
          const bytes = new Uint8Array(await rom);
          FS.mkdirTree("/roms");
          FS.writeFile("/roms/vroom.bin", bytes);
          if (!window.Module.ccall("chg_web_load", "number", ["string"], ["/roms/vroom.bin"]))
            throw new Error("The bundled game could not be loaded.");
          // The real firmware spends ~0.7 s initializing its LCD. Avoid
          // exposing uninitialized video RAM or accepting inputs during boot.
          await new Promise(resolve => setTimeout(resolve, 1200));
          if (document.body.dataset.state === "error") return;
          ready = true;
          controls.forEach(button => { button.disabled = false; });
          document.title = "Vroom — Shift King";
          status.textContent = "Ready to race · " + config.version;
          document.body.dataset.state = "ready";
          canvas.focus({preventScroll:true});
        } catch (error) { fail(error); }
      });
    }]
  };
  const runtime = document.createElement("script");
  runtime.src = "vendor/CHGame_Emulator.js?v=" + config.engineHash;
  runtime.onerror = () => fail(new Error("The emulator download failed."));
  document.body.appendChild(runtime);
})();
