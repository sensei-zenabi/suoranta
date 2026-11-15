import { marked } from './vendor/marked.mjs';

const content = document.querySelector('#news-content');

try {
  const response = await fetch('news.md', { cache: 'no-cache' });
  if (!response.ok) throw new Error(`News request failed (${response.status})`);

  const html = marked.parse(await response.text());
  const news = window.DOMPurify.sanitize(html, {
    USE_PROFILES: { html: true },
    RETURN_DOM_FRAGMENT: true,
  });
  if (news.childNodes.length) content.replaceChildren(news);
} catch (error) {
  console.warn('Unable to render the news.', error);
} finally {
  content.removeAttribute('aria-busy');
}
