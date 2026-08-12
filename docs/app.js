const classnames = [
  { name: "geb_Aliencrash", type: "Encounter", description: "Active UFO wreck with effects, audio, and alien spawning." },
  { name: "StaticObj_geb_Aliencrash", type: "Encounter", description: "Static visual UFO wreck without scripted encounter behavior." },
  { name: "geb_GreenAlien", type: "Creature", description: "Hostile Little Green Man." },
  { name: "geb_GreenAlienMeat", type: "Food", description: "Area 51 Steak harvested from an alien." },
  { name: "geb_GreenAlienSkin", type: "Material", description: "Roswell Hide harvested from an alien." },
  { name: "geb_PlasmaRifle", type: "Weapon", description: "Semi-automatic Montauk Rifle." },
  { name: "geb_PlasmaCartridge", type: "Magazine", description: "15-round Montauk Cartridge." },
  { name: "geb_FoilHat", type: "Clothing", description: "Tin Foil Hat headgear." }
];

const tabButtons = [...document.querySelectorAll("[data-tab]")];
const tabPanels = [...document.querySelectorAll(".tab-panel")];
const tabList = document.querySelector(".tab-list");
const navToggle = document.querySelector(".nav-toggle");
const toast = document.querySelector("#toast");
let toastTimer;

function activateTab(id, updateHash = true) {
  const selectedButton = tabButtons.find((button) => button.dataset.tab === id);
  const selectedPanel = document.getElementById(id);
  if (!selectedButton || !selectedPanel) return;

  tabButtons.forEach((button) => {
    const active = button === selectedButton;
    button.classList.toggle("active", active);
    button.setAttribute("aria-selected", String(active));
    button.tabIndex = active ? 0 : -1;
  });

  tabPanels.forEach((panel) => {
    const active = panel === selectedPanel;
    panel.classList.toggle("active", active);
    panel.hidden = !active;
  });

  tabList.classList.remove("open");
  navToggle.setAttribute("aria-expanded", "false");
  if (updateHash) history.replaceState(null, "", `#${id}`);
  window.scrollTo({ top: document.querySelector(".site-nav").offsetTop, behavior: "smooth" });
}

tabButtons.forEach((button, index) => {
  button.addEventListener("click", () => activateTab(button.dataset.tab));
  button.addEventListener("keydown", (event) => {
    if (!["ArrowLeft", "ArrowRight", "Home", "End"].includes(event.key)) return;
    event.preventDefault();
    let nextIndex = index;
    if (event.key === "ArrowLeft") nextIndex = (index - 1 + tabButtons.length) % tabButtons.length;
    if (event.key === "ArrowRight") nextIndex = (index + 1) % tabButtons.length;
    if (event.key === "Home") nextIndex = 0;
    if (event.key === "End") nextIndex = tabButtons.length - 1;
    tabButtons[nextIndex].focus();
    activateTab(tabButtons[nextIndex].dataset.tab);
  });
});

document.querySelectorAll("[data-open-tab]").forEach((button) => {
  button.addEventListener("click", () => activateTab(button.dataset.openTab));
});

navToggle.addEventListener("click", () => {
  const open = tabList.classList.toggle("open");
  navToggle.setAttribute("aria-expanded", String(open));
});

document.querySelectorAll("[data-install]").forEach((button) => {
  button.addEventListener("click", () => {
    const target = button.dataset.install;
    document.querySelectorAll("[data-install]").forEach((item) => item.classList.toggle("active", item === button));
    document.querySelectorAll("[data-install-panel]").forEach((panel) => {
      const active = panel.dataset.installPanel === target;
      panel.classList.toggle("active", active);
      panel.hidden = !active;
    });
  });
});

function showToast(message = "Copied to clipboard") {
  clearTimeout(toastTimer);
  toast.textContent = message;
  toast.classList.add("show");
  toastTimer = setTimeout(() => toast.classList.remove("show"), 1800);
}

async function copyText(value) {
  try {
    await navigator.clipboard.writeText(value);
    showToast();
  } catch {
    const input = document.createElement("textarea");
    input.value = value;
    input.style.position = "fixed";
    input.style.opacity = "0";
    document.body.append(input);
    input.select();
    document.execCommand("copy");
    input.remove();
    showToast();
  }
}

document.querySelectorAll("[data-copy]").forEach((button) => {
  button.addEventListener("click", () => copyText(button.dataset.copy));
});

document.querySelectorAll("[data-copy-target]").forEach((button) => {
  button.addEventListener("click", () => {
    const target = document.getElementById(button.dataset.copyTarget);
    if (target) copyText(target.textContent.trim());
  });
});

const classnameBody = document.querySelector("#classname-body");
const searchInput = document.querySelector("#classname-search");
const filterSelect = document.querySelector("#classname-filter");
const resultCount = document.querySelector("#result-count");
const emptyState = document.querySelector("#empty-state");

function renderClassnames() {
  const query = searchInput.value.trim().toLowerCase();
  const type = filterSelect.value;
  const results = classnames.filter((item) => {
    const matchesText = `${item.name} ${item.type} ${item.description}`.toLowerCase().includes(query);
    return matchesText && (type === "all" || item.type === type);
  });

  classnameBody.replaceChildren(...results.map((item) => {
    const row = document.createElement("tr");
    const name = document.createElement("td");
    const itemType = document.createElement("td");
    const description = document.createElement("td");
    const action = document.createElement("td");
    const chip = document.createElement("span");
    const copy = document.createElement("button");

    name.textContent = item.name;
    chip.className = "type-chip";
    chip.textContent = item.type;
    itemType.append(chip);
    description.textContent = item.description;
    copy.className = "row-copy";
    copy.type = "button";
    copy.textContent = "Copy";
    copy.setAttribute("aria-label", `Copy ${item.name}`);
    copy.addEventListener("click", () => copyText(item.name));
    action.append(copy);
    row.append(name, itemType, description, action);
    return row;
  }));

  resultCount.textContent = `${results.length} ${results.length === 1 ? "entry" : "entries"}`;
  emptyState.hidden = results.length !== 0;
  document.querySelector(".table-wrap").hidden = results.length === 0;
}

searchInput.addEventListener("input", renderClassnames);
filterSelect.addEventListener("change", renderClassnames);
renderClassnames();

const initialTab = location.hash.slice(1);
if (tabButtons.some((button) => button.dataset.tab === initialTab)) activateTab(initialTab, false);

window.addEventListener("hashchange", () => {
  const id = location.hash.slice(1);
  if (tabButtons.some((button) => button.dataset.tab === id)) activateTab(id, false);
});

/* --- Ambient effects ------------------------------------------------- */

const reducedMotion = window.matchMedia("(prefers-reduced-motion: reduce)").matches;

// Twinkling starfield with occasional shooting stars.
const starCanvas = document.getElementById("starfield");
if (starCanvas && !reducedMotion) {
  const ctx = starCanvas.getContext("2d");
  let stars = [];
  let shootingStar = null;
  let nextShootingStar = performance.now() + 4000 + Math.random() * 6000;
  let width = 0;
  let height = 0;

  function resizeStarfield() {
    const dpr = Math.min(window.devicePixelRatio || 1, 2);
    width = starCanvas.clientWidth;
    height = starCanvas.clientHeight;
    starCanvas.width = width * dpr;
    starCanvas.height = height * dpr;
    ctx.setTransform(dpr, 0, 0, dpr, 0, 0);
    const count = Math.min(220, Math.round((width * height) / 6500));
    stars = Array.from({ length: count }, () => ({
      x: Math.random() * width,
      y: Math.random() * height,
      radius: Math.random() * 1.3 + 0.3,
      phase: Math.random() * Math.PI * 2,
      speed: 0.4 + Math.random() * 1.4,
      drift: 0.008 + Math.random() * 0.03,
      tint: Math.random() < 0.16 ? "156, 255, 190" : Math.random() < 0.5 ? "170, 235, 255" : "230, 244, 238"
    }));
  }

  function spawnShootingStar() {
    const fromLeft = Math.random() < 0.5;
    shootingStar = {
      x: fromLeft ? -40 : width + 40,
      y: Math.random() * height * 0.45,
      vx: (fromLeft ? 1 : -1) * (7 + Math.random() * 5),
      vy: 1.6 + Math.random() * 1.6,
      life: 1
    };
  }

  function drawStarfield(now) {
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

    if (!shootingStar && now > nextShootingStar) spawnShootingStar();
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
    requestAnimationFrame(drawStarfield);
  }

  resizeStarfield();
  window.addEventListener("resize", resizeStarfield);
  requestAnimationFrame(drawStarfield);
}

// Scrambled-transmission decode effect on the hero status line.
const decodeLine = document.getElementById("decode-line");
if (decodeLine && !reducedMotion) {
  const finalText = decodeLine.textContent;
  const glyphs = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789#$%&/\\<>*+=";
  let frame = 0;
  const totalFrames = 46;
  decodeLine.textContent = "";
  const decoder = setInterval(() => {
    frame += 1;
    const solved = Math.floor((frame / totalFrames) * finalText.length);
    decodeLine.textContent = finalText
      .split("")
      .map((char, index) => {
        if (index < solved || char === " " || char === "/") return char;
        return glyphs[Math.floor(Math.random() * glyphs.length)];
      })
      .join("");
    if (frame >= totalFrames) {
      decodeLine.textContent = finalText;
      clearInterval(decoder);
    }
  }, 34);
}

// Reveal cards and steps as they scroll (or tab) into view.
const revealTargets = document.querySelectorAll(
  ".stat-card, .feature-card, .loadout-card, .info-card, .timeline article, .steps li, .support-grid a, .ce-step, .callout"
);
if (revealTargets.length && !reducedMotion && "IntersectionObserver" in window) {
  const revealObserver = new IntersectionObserver((entries) => {
    entries.forEach((entry) => {
      if (!entry.isIntersecting) return;
      entry.target.classList.add("fx-in");
      revealObserver.unobserve(entry.target);
    });
  }, { threshold: 0.15 });

  revealTargets.forEach((element, index) => {
    element.classList.add("fx-reveal");
    element.style.transitionDelay = `${(index % 4) * 70}ms`;
    revealObserver.observe(element);
  });
}
