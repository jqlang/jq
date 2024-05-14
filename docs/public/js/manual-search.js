(() => {
  const searchInput = document.querySelector('input#searchbox');
  const sectionIDs = JSON.parse(document.querySelector('#section-ids').innerText);
  const sanitize = (string) => string.replaceAll('<', '&lt;').replaceAll('>', '&gt;');
  new autoComplete({
    selector: `#${searchInput.id}`,
    wrapper: false,
    data: {
      src: Object.keys(sectionIDs),
      filter: (list) => list.sort((x, y) =>
        x.match.indexOf('<') - y.match.indexOf('<') || x.value.length - y.value.length),
    },
    searchEngine: (query, value) => {
      const index = value.toLowerCase().indexOf(query.toLowerCase());
      if (index >= 0) {
        return sanitize(value.substring(0, index)) +
          `<mark>${sanitize(value.substring(index, index + query.length))}</mark>` +
          sanitize(value.substring(index + query.length));
      }
    },
  });
  searchInput.addEventListener('selection', (event) => {
    event.target.value = event.detail.selection.value;
    location.hash = `#${sectionIDs[event.detail.selection.value]}`;
  });
  document.addEventListener('keydown', (event) => {
    if (event.code === 'Slash' && !event.altKey && !event.ctrlKey && !event.metaKey
      && !event.shiftKey && !/^(INPUT|TEXTAREA)$/.test(event.target.nodeName)) {
      searchInput.focus();
      searchInput.select();
      event.preventDefault();
    }
  });
})();
