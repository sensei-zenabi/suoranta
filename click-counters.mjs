const counterApi = 'https://budostack-download-clicks.firmcheetah8.chatgpt.site/v1/clicks';
const countFormat = new Intl.NumberFormat('en');

export function setupClickCounters(root = document, request = fetch) {
  const entries = [...root.querySelectorAll('[data-asset]')].map(metadata => {
    const row = metadata.closest('.download');
    const button = row.querySelector('a');
    const stats = root.createElement('span');
    stats.className = 'file-info';
    stats.textContent = 'Loading clicks…';
    stats.title = 'Download button clicks, including repeat clicks. Not unique people or completed downloads.';
    row.append(stats);
    return {app: metadata.dataset.release, button, stats, count: null};
  });

  function show(entry, count) {
    if (!Number.isSafeInteger(count) || count < 0) return;
    entry.count = Math.max(entry.count ?? 0, count);
    entry.stats.textContent = `${countFormat.format(entry.count)} ${entry.count === 1 ? 'click' : 'clicks'}`;
  }

  async function record(entry, event) {
    // Trusted click includes mouse, touch, and keyboard activation. Middle click
    // fires auxclick; right-click only opens a menu and is not a download attempt.
    if (!event.isTrusted || (event.type === 'auxclick' ? event.button !== 1 : event.button !== 0)) return;
    const id = crypto.randomUUID();
    const options = {method: 'POST', headers: {'Content-Type': 'text/plain'}, body: JSON.stringify({event: id}), keepalive: true, credentials: 'omit'};
    for (let attempt = 0; attempt < 2; attempt += 1) {
      try {
        const response = await request(`${counterApi}/${entry.app}`, options);
        if (!response.ok) throw new Error('Counter unavailable');
        const result = await response.json();
        show(entry, result.count);
        return;
      } catch {
        // Retry with the same event ID so a lost response cannot double count.
      }
    }
    entry.stats.textContent = 'Clicks unavailable';
  }

  for (const entry of entries) {
    entry.button.addEventListener('click', event => { void record(entry, event); });
    entry.button.addEventListener('auxclick', event => { void record(entry, event); });
  }
  // Loading statistics never records a click. Do not block the download's
  // normal browser action while the keepalive request is being recorded.
  return request(counterApi, {cache: 'no-store', credentials: 'omit'})
    .then(response => {
      if (!response.ok) throw new Error('Counter unavailable');
      return response.json();
    })
    .then(({counts}) => { for (const entry of entries) show(entry, counts[entry.app]); })
    .catch(() => { for (const entry of entries) if (entry.count === null) entry.stats.textContent = 'Clicks unavailable'; });
}

if (typeof document !== 'undefined') void setupClickCounters();
