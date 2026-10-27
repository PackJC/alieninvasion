// Alien Invasion's site script: the Geb's family navigation, scroll reveals and copy toast, plus this site's own
// starfield hero, decoding status line, XML colouring, install switch and classname table. The sections keep the
// old tab names as ids, so links like #install and #classnames still land in the right place.
(() => {
  "use strict";
  const $ = (s, r = document) => r.querySelector(s);
  const $$ = (s, r = document) => [...r.querySelectorAll(s)];
  document.documentElement.classList.add("js");
  const reducedMotion = window.matchMedia("(prefers-reduced-motion: reduce)").matches;

  const classnames = [
    { name: "geb_Aliencrash", type: "Encounter", img: "geb_Aliencrash", description: "Active UFO wreck: effects, audio, toxic gas, salvage, and alien spawning." },
    { name: "StaticObj_geb_Aliencrash", type: "Encounter", img: "geb_Aliencrash", description: "Static visual UFO wreck without scripted encounter behavior." },
    { name: "geb_AlienRadiationArea", type: "Encounter", description: "8 m toxic cloud each active wreck creates and removes; not meant to be spawned on its own." },
    { name: "geb_GreenAlien", type: "Creature", img: "geb_GreenAlien", description: "Hostile Little Green Man with melee and a psychic zap." },
    { name: "geb_AlienTank", type: "Prop", img: "geb_AlienTank", description: "Specimen Tank: a sleeping alien in glowing green fluid. A prop: place it with an admin tool or editor." },
    { name: "geb_GreenAlienMeat", type: "Food", img: "geb_GreenAlienMeat", description: "Area 51 Steak from skinning; cooked gives night vision, bad meat poisons." },
    { name: "geb_GreenAlienSkin", type: "Material", img: "geb_GreenAlienSkin", description: "Roswell Hide from skinning an alien; tans into Roswell Leather." },
    { name: "geb_GreenAlienLeather", type: "Material", img: "geb_GreenAlienLeather", description: "Roswell Leather: a hide tanned with garden lime, by hand or in a barrel." },
    { name: "geb_GreenAlienLeatherSack", type: "Backpack", img: "geb_GreenAlienLeatherSack", description: "Roswell Leather Backpack: two Roswell Leather and a leather sewing kit." },
    { name: "geb_GreenAlienCourierBag", type: "Backpack", img: "geb_GreenAlienCourierBag", description: "Roswell Hide Courier Bag: a hide and a rope." },
    { name: "geb_GreenAlienImprovisedBag", type: "Backpack", img: "geb_GreenAlienImprovisedBag", description: "Roswell Hide Backpack: the courier bag on a frame of three sticks." },
    { name: "geb_PlasmaRifle", type: "Weapon", img: "geb_PlasmaRifle", description: "Semi-automatic Montauk Rifle; plasma bolts in the cell's colour, double damage to aliens. Stock, iron sights, live read-out." },
    { name: "geb_PlasmaCartridge", type: "Magazine", img: "geb_PlasmaCartridge", description: "15-round Montauk Cartridge; recharge with a 9V battery. Green plasma." },
    { name: "geb_PlasmaCartridge_Arc", type: "Magazine", img: "geb_PlasmaCartridge_Arc", description: "Montauk Cartridge (Blue): lightning on the muzzle, bouncing sparks." },
    { name: "geb_PlasmaCartridge_Lance", type: "Magazine", img: "geb_PlasmaCartridge_Lance", description: "Montauk Cartridge (Lime): a long lance, shock rings, yellow bolt." },
    { name: "geb_PlasmaCartridge_Void", type: "Magazine", img: "geb_PlasmaCartridge_Void", description: "Montauk Cartridge (Violet): an arc vortex, floating sparks, violet bolt." },
    { name: "Ammo_Plasma_Cell", type: "Ammunition", img: "geb_PlasmaCartridge", description: "Loose Plasma Cell from clearing a loaded rifle's chamber; loads back into any cartridge. Never spawns as loot." },
    { name: "geb_PlasmaGrenade", type: "Explosive", img: "geb_PlasmaGrenade", description: "Plasma Grenade: sticks to what it hits and bursts 3 s after leaving the hand. Crash sites and some aliens carry one." },
    { name: "geb_FoilHat", type: "Clothing", img: "geb_FoilHat", description: "Tin Foil Hat; aliens can't track or zap the wearer." }
  ];

  // ---- copy + toast
  const toast = $("#toast");
  let toastTimer;
  async function copy(text, label = "Copied to clipboard") {
    try { await navigator.clipboard.writeText(text); }
    catch {
      const t = document.createElement("textarea");
      t.value = text;
      t.style.cssText = "position:fixed;opacity:0";
      document.body.append(t); t.select(); document.execCommand("copy"); t.remove();
    }
    toast.textContent = label;
    toast.classList.add("show");
    clearTimeout(toastTimer);
    toastTimer = setTimeout(() => toast.classList.remove("show"), 1500);
  }
  document.addEventListener("click", e => {
    const block = e.target.closest("[data-copy]");
    if (block) copy($("#" + block.dataset.copy).textContent.trim());
    const cls = e.target.closest("[data-cls]");
    if (cls) copy(cls.dataset.cls, cls.dataset.cls + " copied");
  });

  // ---- navigation: the menu on small screens, and the current section lit up in the bar
  const menu = $("#menu"), links = $("#nav-links");
  menu.addEventListener("click", () => {
    const open = links.classList.toggle("open");
    menu.setAttribute("aria-expanded", String(open));
  });
  $$("a", links).forEach(a => a.addEventListener("click", () => {
    links.classList.remove("open"); menu.setAttribute("aria-expanded", "false");
  }));
  const navMap = new Map($$("a[href^='#']", links).map(a => [a.getAttribute("href").slice(1), a]));
  const spy = new IntersectionObserver(entries => {
    for (const en of entries) {
      if (!en.isIntersecting) continue;
      navMap.forEach(a => a.classList.remove("active"));
      const a = navMap.get(en.target.id);
      if (a) a.classList.add("active");
    }
  }, { rootMargin: "-45% 0px -50% 0px" });
  $$("main > section[id]").forEach(s => spy.observe(s));

  // ---- reveal on scroll
  const reveal = new IntersectionObserver(entries => {
    for (const en of entries) if (en.isIntersecting) { en.target.classList.add("in"); reveal.unobserve(en.target); }
  }, { rootMargin: "0px 0px -8% 0px" });
  const watch = root => $$(".reveal:not(.in)", root).forEach(el => reveal.observe(el));
  $$(".grid.items, .tiles").forEach(grid => [...grid.children].forEach((c, i) => {
    c.style.transitionDelay = Math.min(i % 4, 3) * 60 + "ms";
  }));
  watch(document);

  // ---- install: player or server owner
  const installButtons = $$("[data-install]");
  installButtons.forEach(button => button.addEventListener("click", () => {
    installButtons.forEach(b => b.setAttribute("aria-pressed", String(b === button)));
    $$("[data-install-panel]").forEach(panel => {
      panel.hidden = panel.dataset.installPanel !== button.dataset.install;
      watch(panel);
    });
  }));

  // ---- light XML colouring for code blocks marked .language-xml (copy reads textContent, which the spans keep)
  const escapeHtml = text => text.replace(/&/g, "&amp;").replace(/</g, "&lt;").replace(/>/g, "&gt;");
  function highlightXml(code) {
    const source = code.textContent;
    const tokenPattern = /(<!--[\s\S]*?-->)|(<\/?)([\w:.-]+)((?:\s+[\w:.-]+(?:="[^"]*")?)*)(\s*\/?>)/g;
    const span = (cls, text) => `<span class="${cls}">${escapeHtml(text)}</span>`;
    let html = "", last = 0, match;
    while ((match = tokenPattern.exec(source))) {
      html += escapeHtml(source.slice(last, match.index));
      if (match[1]) {
        html += span("tok-com", match[1]);
      } else {
        const attributes = match[4].replace(/([\w:.-]+)(="[^"]*")?/g, (_, name, value) =>
          span("tok-attr", name) + (value ? span("tok-punct", "=") + span("tok-val", value.slice(1)) : ""));
        html += span("tok-punct", match[2]) + span("tok-tag", match[3]) + attributes + span("tok-punct", match[5]);
      }
      last = tokenPattern.lastIndex;
    }
    code.innerHTML = html + escapeHtml(source.slice(last));
  }
  $$("code.language-xml").forEach(highlightXml);

  // ---- classnames
  const body = $("#classname-body"), search = $("#classname-search"), filter = $("#classname-filter");
  const count = $("#result-count"), empty = $("#empty-state"), table = $("#classname-table");
  function renderClassnames() {
    const query = search.value.trim().toLowerCase();
    const type = filter.value;
    const results = classnames.filter(item =>
      `${item.name} ${item.type} ${item.description}`.toLowerCase().includes(query) && (type === "all" || item.type === type));

    body.replaceChildren(...results.map(item => {
      const row = document.createElement("tr");
      const name = document.createElement("td");
      const cell = document.createElement("div");
      cell.className = "name-cell";
      const thumb = document.createElement("span");
      thumb.className = "thumb";
      if (item.img) {
        const img = document.createElement("img");
        img.src = `items/${item.img}.webp`;
        img.alt = "";
        img.loading = "lazy";
        thumb.append(img);
      }
      const button = document.createElement("button");
      button.className = "cls";
      button.type = "button";
      button.title = "Copy classname";
      button.dataset.cls = item.name;
      button.textContent = item.name;
      cell.append(thumb, button);
      name.append(cell);
      const kind = document.createElement("td");
      const chip = document.createElement("span");
      chip.className = "chip";
      chip.textContent = item.type;
      kind.append(chip);
      const description = document.createElement("td");
      description.textContent = item.description;
      row.append(name, kind, description);
      return row;
    }));

    count.textContent = `${results.length} ${results.length === 1 ? "classname" : "classnames"}`;
    empty.hidden = results.length !== 0;
    table.hidden = results.length === 0;
  }
  search.addEventListener("input", renderClassnames);
  filter.addEventListener("change", renderClassnames);
  renderClassnames();

  // ---- the hero's sky: twinkling stars drifting left, and a shooting star now and then
  const sky = $("#starfield");
  if (sky && !reducedMotion) {
    const ctx = sky.getContext("2d");
    let stars = [], shootingStar = null, width = 0, height = 0, visible = true;
    let nextShootingStar = performance.now() + 4000 + Math.random() * 6000;

    function resize() {
      const dpr = Math.min(window.devicePixelRatio || 1, 2);
      width = sky.clientWidth;
      height = sky.clientHeight;
      sky.width = width * dpr;
      sky.height = height * dpr;
      ctx.setTransform(dpr, 0, 0, dpr, 0, 0);
      const n = Math.min(220, Math.round((width * height) / 6500));
      stars = Array.from({ length: n }, () => ({
        x: Math.random() * width,
        y: Math.random() * height,
        radius: Math.random() * 1.3 + 0.3,
        phase: Math.random() * Math.PI * 2,
        speed: 0.4 + Math.random() * 1.4,
        drift: 0.008 + Math.random() * 0.03,
        tint: Math.random() < 0.16 ? "156, 255, 190" : Math.random() < 0.5 ? "170, 235, 255" : "230, 244, 238"
      }));
    }

    function draw(now) {
      if (visible) {
        ctx.clearRect(0, 0, width, height);
        for (const star of stars) {
          const twinkle = 0.32 + 0.68 * (0.5 + 0.5 * Math.sin(star.phase + now * 0.001 * star.speed));
          star.x -= star.drift;
          if (star.x < -2) star.x = width + 2;
          ctx.beginPath();
          ctx.arc(star.x, star.y, star.radius, 0, Math.PI * 2);
          ctx.fillStyle = `rgba(${star.tint}, ${twinkle})`;
          ctx.fill();
        }
        if (!shootingStar && now > nextShootingStar) {
          const fromLeft = Math.random() < 0.5;
          shootingStar = { x: fromLeft ? -40 : width + 40, y: Math.random() * height * 0.45,
            vx: (fromLeft ? 1 : -1) * (7 + Math.random() * 5), vy: 1.6 + Math.random() * 1.6, life: 1 };
        }
        if (shootingStar) {
          const s = shootingStar;
          const tail = ctx.createLinearGradient(s.x, s.y, s.x - s.vx * 9, s.y - s.vy * 9);
          tail.addColorStop(0, `rgba(190, 255, 210, ${0.9 * s.life})`);
          tail.addColorStop(1, "rgba(190, 255, 210, 0)");
          ctx.strokeStyle = tail;
          ctx.lineWidth = 1.6;
          ctx.beginPath();
          ctx.moveTo(s.x, s.y);
          ctx.lineTo(s.x - s.vx * 9, s.y - s.vy * 9);
          ctx.stroke();
          s.x += s.vx;
          s.y += s.vy;
          s.life -= 0.011;
          if (s.life <= 0 || s.x < -120 || s.x > width + 120 || s.y > height + 40) {
            shootingStar = null;
            nextShootingStar = now + 5000 + Math.random() * 9000;
          }
        }
      }
      requestAnimationFrame(draw);
    }

    // Only draw while the hero is on screen
    new IntersectionObserver(entries => { visible = entries[0].isIntersecting; }).observe(sky);
    resize();
    window.addEventListener("resize", resize);
    requestAnimationFrame(draw);
  }

  // ---- the status line decodes like an incoming transmission
  const decodeLine = $("#decode-line");
  if (decodeLine && !reducedMotion) {
    const finalText = decodeLine.textContent;
    const glyphs = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789#$%&/\\<>*+=";
    const totalFrames = 46;
    let frame = 0;
    decodeLine.textContent = "";
    const decoder = setInterval(() => {
      frame += 1;
      const solved = Math.floor((frame / totalFrames) * finalText.length);
      decodeLine.textContent = finalText.split("").map((char, index) =>
        index < solved || char === " " || char === "/" ? char : glyphs[Math.floor(Math.random() * glyphs.length)]).join("");
      if (frame >= totalFrames) {
        decodeLine.textContent = finalText;
        clearInterval(decoder);
      }
    }, 34);
  }
})();
