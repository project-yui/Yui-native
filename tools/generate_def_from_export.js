const fs = require('fs');
const path = require('path');

if (process.argv.length < 4) {
  console.error('Usage: node generate_def_from_export.js <export_list.txt> <out.def> [dllname]');
  process.exit(2);
}

const inFile = process.argv[2];
const outFile = process.argv[3];
const dllName = process.argv[4] || path.basename(outFile, '.def');

// Read as Buffer and detect encoding (support UTF-16 LE with BOM)
const buf = fs.readFileSync(inFile);
let content;
if (buf.length >= 2 && buf[0] === 0xFF && buf[1] === 0xFE) {
  content = buf.toString('utf16le');
} else {
  // fallback to utf8
  content = buf.toString('utf8');
}
const lines = content.split(/\r?\n/);

const names = [];
for (let i = 0; i < lines.length; ++i) {
  const line = lines[i];
  const m = line.match(/^\s*Name\s*:\s*(.*)$/);
  if (!m) continue;
  // Start with the rest of the line after 'Name :'
  let mangled = m[1] || '';
  // If the mangled name is wrapped across lines, keep consuming until we hit a line
  // that looks like the VA : marker or the next Export header.
  let j = i + 1;
  while (j < lines.length) {
    const nl = lines[j];
    // If this line contains the VA : marker anywhere, stop consuming continued name pieces
    if (/\bVA\s*:/.test(nl)) break;
    if (/^\s*Export\s+\d+\s*:/.test(nl)) break;
    // Append the continued piece (trim leading/trailing whitespace)
    mangled += (nl || '').trim();
    j++;
  }
  // Normalize whitespace inside the mangled name (remove spaces/newlines introduced by wrapping)
  mangled = mangled.replace(/\s+/g, '');
  if (mangled.length > 0) names.push(mangled);
  // advance i to j-1 so the outer loop continues after the consumed lines
  i = j - 1;
}

const out = [];
out.push('LIBRARY "' + dllName + '.dll"');
out.push('EXPORTS');
// The forwarder syntax expects the target DLL name without the .dll extension.
// Avoid using '.' in the module name because the linker splits the first '.' as the
// separator between module and symbol (causing e.g. unresolved "original.<sym>").
// Use an underscore to form a valid single token like "QQNT_original" which
// corresponds to "QQNT_original.dll" at runtime.
const targetPrefix = dllName + '.original';
for (const n of names) {
  if (n.length === 0) continue;
  // forward to original: <mangled>=<dllname>.original.<mangled>
  out.push('    ' + n + '=' + targetPrefix + '.' + n);
}

fs.writeFileSync(outFile, out.join('\n'), 'utf8');
console.log('Wrote', outFile, 'with', names.length, 'exports');
