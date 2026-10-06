// Render the host SVG previews with sharp and create a labeled comparison sheet.
const fs = require('fs');
const path = require('path');
const sharp = require(process.env.SHARP_MODULE || 'sharp');
async function main() {
  const dir = process.argv[2] || 'docs/previews';
  const names = ['book-single', 'book-spread', 'clean', 'oled-night'];
  const labels = ['BOOK · UNE PAGE', 'BOOK · DEUX PAGES', 'CLEAN', 'OLED NIGHT'];
  const composite = [];
  for (let i = 0; i < names.length; i++) {
    const png = path.join(dir, names[i] + '.png');
    await sharp(fs.readFileSync(path.join(dir, names[i] + '.svg'))).png().toFile(png);
    composite.push({ input: await sharp(png).resize(768, 435).toBuffer(), left: (i % 2) * 792 + 12, top: Math.floor(i / 2) * 490 + 42 });
    const label = Buffer.from(`<svg width="768" height="28"><text x="0" y="21" font-family="Arial" font-size="16" fill="#b7bec4">${labels[i]} — aperçu hôte</text></svg>`);
    composite.push({ input: label, left: (i % 2) * 792 + 12, top: Math.floor(i / 2) * 490 + 10 });
  }
  await sharp({ create: { width: 1584, height: 980, channels: 4, background: '#101419' } }).composite(composite).png().toFile(path.join(dir, 'reading-modes.png'));
}
main().catch(error => { console.error(error); process.exit(1); });
