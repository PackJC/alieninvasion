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
