const releaseApi = "https://api.github.com/repos/sensei-zenabi/suoranta/releases";
const releaseTag = "budostack-packages";
const downloadBase = "https://github.com/sensei-zenabi/suoranta/releases/download";
const assetName = /^(?:[a-z][a-z0-9]{0,7}|[a-z0-9]+(?:-[a-z0-9]+)*-\d+\.\d+\.\d+-dos)-[0-9a-f]{64}\.zip$/;
const dateFormat = new Intl.DateTimeFormat("en-CA", {
  timeZone: "Europe/Helsinki",
  year: "numeric",
  month: "2-digit",
  day: "2-digit",
});

async function getJson(url) {
  const response = await fetch(url);
  if (!response.ok) throw new Error(`GitHub returned ${response.status}`);
  return response.json();
}

async function loadDownloads() {
  const entries = [...document.querySelectorAll("[data-asset]")].map((metadata) => {
    const row = metadata.closest(".download");
    const button = row.querySelector("a");
    return { metadata, button, stats: row.querySelector("[data-download-stats]") };
  });

  const assets = new Map();
  const releaseResponse = await fetch(`${releaseApi}/tags/${releaseTag}`);
  if (releaseResponse.status === 404) return;
  if (!releaseResponse.ok) throw new Error(`GitHub returned ${releaseResponse.status}`);
  const release = await releaseResponse.json();
  for (let page = 1; ; page += 1) {
    const batch = await getJson(`${releaseApi}/${release.id}/assets?per_page=100&page=${page}`);
    for (const asset of batch) {
      const match = assetName.exec(asset.name);
      if (!match || asset.state !== "uploaded" || assets.has(asset.name)) continue;
      assets.set(asset.name, asset);
    }
    if (batch.length < 100) break;
  }

  for (const { metadata, button, stats } of entries) {
    const asset = assets.get(metadata.dataset.asset);
    if (!asset) continue;
    button.href = `${downloadBase}/${releaseTag}/${metadata.dataset.asset}`;
    const uploaded = document.createElement("time");
    uploaded.dateTime = asset.created_at;
    uploaded.textContent = `Uploaded ${dateFormat.format(new Date(asset.created_at))}`;
    stats.append(uploaded);
    stats.hidden = false;
  }
}

loadDownloads().catch(() => {
  // The site downloads remain usable when the release asset API fails.
});
