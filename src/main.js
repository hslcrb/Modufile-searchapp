const { invoke } = window.__TAURI__.core;

let searchInput;
let resultsList;
let statusText;
let refreshBtn;
let langSelect;
let themeToggle;
let smartMatchCheck;
let smartMatchLabel;
let currentLanguage = "ko";

const translations = {
  ko: {
    title: "모두파일",
    placeholder: "파일 검색...",
    refresh: "인덱싱 새로고침",
    waiting: "대기 중",
    indexing: "인덱싱 중...",
    complete: (count) => `발견: ${count.toLocaleString()}`,
    found: (count) => `${count}개`,
    noResults: "결과 없음",
    smartMatch: "알잘딱"
  },
  en: {
    title: "Modufile",
    placeholder: "Search...",
    refresh: "Refresh",
    waiting: "Waiting",
    indexing: "Indexing...",
    complete: (count) => `Found: ${count.toLocaleString()}`,
    found: (count) => `${count} files`,
    noResults: "No results",
    smartMatch: "Sensible Search"
  },
  ja: {
    title: "モドゥ",
    placeholder: "検索...",
    refresh: "更新",
    waiting: "待機中",
    indexing: "読込中...",
    complete: (count) => `発見: ${count.toLocaleString()}`,
    found: (count) => `${count}件`,
    noResults: "結果なし",
    smartMatch: "察しがいい検索"
  },
  "zh-TW": {
    title: "全檔案",
    placeholder: "搜尋...",
    refresh: "重新整理",
    waiting: "等待中",
    indexing: "索引中...",
    complete: (count) => `發現: ${count.toLocaleString()}`,
    found: (count) => `找到 ${count} 個`,
    noResults: "查無結果",
    smartMatch: "心領神會搜尋"
  }
};

function initTheme() {
  const savedTheme = localStorage.getItem("modufile_theme") || "dark";
  document.documentElement.setAttribute("data-theme", savedTheme);
}

function getSystemLanguage() {
  const lang = navigator.language.toLowerCase();
  if (lang.startsWith("ko")) return "ko";
  if (lang.startsWith("ja")) return "ja";
  if (lang.startsWith("zh-tw") || lang.startsWith("zh-hk")) return "zh-TW";
  return "en";
}

async function init() {
  searchInput = document.querySelector("#search-input");
  resultsList = document.querySelector("#results-list");
  statusText = document.querySelector("#status-text");
  refreshBtn = document.querySelector("#refresh-btn");
  langSelect = document.querySelector("#lang-select");
  themeToggle = document.querySelector("#theme-toggle");
  smartMatchCheck = document.querySelector("#smart-match-check");
  smartMatchLabel = document.querySelector("#smart-match-label");

  // Language setup
  const savedLang = localStorage.getItem("modufile_lang");
  currentLanguage = savedLang || getSystemLanguage();
  langSelect.value = currentLanguage;

  // Theme setup
  initTheme();

  updateUIStrings();

  langSelect.addEventListener("change", (e) => {
    currentLanguage = e.target.value;
    localStorage.setItem("modufile_lang", currentLanguage);
    updateUIStrings();
  });

  themeToggle.addEventListener("click", () => {
    const current = document.documentElement.getAttribute("data-theme");
    const next = current === "dark" ? "light" : "dark";
    document.documentElement.setAttribute("data-theme", next);
    localStorage.setItem("modufile_theme", next);
  });

  searchInput.addEventListener("input", performSearch);
  smartMatchCheck.addEventListener("change", performSearch);

  refreshBtn.addEventListener("click", refreshIndex);

  await refreshIndex();
}

async function performSearch() {
  const query = searchInput.value;
  const smartMatch = smartMatchCheck.checked;
  const t = translations[currentLanguage];

  try {
    const results = await invoke("search", { query, smartMatch });
    renderResults(results);
    statusText.textContent = t.found(results.length);
  } catch (error) {
    console.error(error);
  }
}

async function refreshIndex() {
  const t = translations[currentLanguage];
  refreshBtn.disabled = true;
  statusText.textContent = t.indexing;

  try {
    const count = await invoke("refresh_index");
    statusText.textContent = t.complete(count);
    await performSearch();
  } catch (error) {
    statusText.textContent = "Error";
  } finally {
    refreshBtn.disabled = false;
  }
}

function updateUIStrings() {
  const t = translations[currentLanguage];
  document.querySelector(".title-text").textContent = t.title;
  searchInput.placeholder = t.placeholder;
  refreshBtn.textContent = t.refresh;
  smartMatchLabel.textContent = t.smartMatch;
  if (statusText.textContent === "" || statusText.textContent.includes("대기")) {
    statusText.textContent = t.waiting;
  }
}

function renderResults(results) {
  const t = translations[currentLanguage];
  resultsList.innerHTML = "";

  if (results.length === 0 && searchInput.value !== "") {
    resultsList.innerHTML = `<div style="text-align: center; opacity: 0.3; margin-top: 50px;">${t.noResults}</div>`;
    return;
  }

  results.forEach(file => {
    const item = document.createElement("div");
    item.className = "result-item";
    item.innerHTML = `
      <div class="result-name">${escapeHtml(file.name)}</div>
      <div class="result-path">${escapeHtml(file.path)}</div>
    `;
    item.addEventListener("click", () => invoke("open_file", { path: file.path }));
    resultsList.appendChild(item);
  });
}

function escapeHtml(text) {
  const div = document.createElement("div");
  div.textContent = text;
  return div.innerHTML;
}

window.addEventListener("DOMContentLoaded", init);
