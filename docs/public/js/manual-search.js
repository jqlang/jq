var section_names = function(q) {
  if (!q) {
    return [];
  }
  var matches = [];
  q = q.toLowerCase();
  $.each(section_map, function(k, v) {
    if (k.toLowerCase().indexOf(q) != -1) {
      matches.push(k);
    }
  });
  matches.sort(function(a, b) {
    // shortest to longest
    return a.length - b.length;
  });
  return matches;
}
var section_names_cb = function(q, cb) {
  cb(section_names(q));
}
var go_to_section = function() {
  query = $('#searchbox').val();
  results = section_names(query);
  if (results.length == 0) {
    return;
  }
  result = results[0];
  location.hash = '#' + section_map[result];
  if (result != query) {
    $('#searchbox').val(result);
  }
}
$(function(){
  $('#searchbox').typeahead(
    {hint: false, highlight: true, minLength: 1},
    {name: "contents", source: section_names_cb, limit: 6}
  ).on('typeahead:selected', function(e, data) {
    go_to_section();
  });
  $('#searchbox').change(go_to_section);
});
// add "Run" button to execute examples on jqplay.org
$(function() {
  $.each($('.manual-example table'), function(index, value) {
    $value = $(value)
    var j = $value.find('tr:nth-child(2) td:first').text();
    var q = $value.find('.jqprogram').text().replace(/^jq /, '').replace(/(\r\n|\n|\r)/gm," ").replace(/^'(.+)'$/, '$1');
    var url = 'https://jqplay.org/jq?q=' + encodeURIComponent(q) +'&j=' + encodeURIComponent(j)
    var $last_tr = $value.find('tr:last');
    $last_tr.after('<tr class="jqplay-btn"><th><a href="' + url + '" class="btn btn-primary btn-sm">Run</a></th><th></th></tr><tr><th></th><th></th></tr>');
  });
});
