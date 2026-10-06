"""Generate an original, redistributable EPUB to exercise the reader on hardware."""
from pathlib import Path
from zipfile import ZipFile, ZIP_STORED, ZIP_DEFLATED

target = Path(__file__).resolve().parents[1] / 'assets' / 'VitaReader-test.epub'
target.parent.mkdir(exist_ok=True)
with ZipFile(target, 'w') as z:
    z.writestr('mimetype', 'application/epub+zip', compress_type=ZIP_STORED)
    z.writestr('META-INF/container.xml', '''<?xml version="1.0"?><container xmlns="urn:oasis:names:tc:opendocument:xmlns:container" version="1.0"><rootfiles><rootfile full-path="EPUB/book.opf" media-type="application/oebps-package+xml"/></rootfiles></container>''', compress_type=ZIP_DEFLATED)
    z.writestr('EPUB/book.opf', '''<?xml version="1.0"?><package xmlns="http://www.idpf.org/2007/opf" version="2.0" unique-identifier="id"><metadata xmlns:dc="http://purl.org/dc/elements/1.1/"><dc:title>Le carnet de lecture</dc:title><dc:creator>VitaReader</dc:creator><dc:language>fr</dc:language><dc:identifier id="id">urn:vitareader:test:1</dc:identifier></metadata><manifest><item id="a" href="chapitre1.xhtml" media-type="application/xhtml+xml"/><item id="b" href="chapitre2.xhtml" media-type="application/xhtml+xml"/><item id="ncx" href="toc.ncx" media-type="application/x-dtbncx+xml"/></manifest><spine toc="ncx"><itemref idref="a"/><itemref idref="b"/></spine></package>''', compress_type=ZIP_DEFLATED)
    z.writestr('EPUB/toc.ncx', '''<?xml version="1.0"?><ncx xmlns="http://www.daisy.org/z3986/2005/ncx/" version="2005-1"><head><meta name="dtb:uid" content="urn:vitareader:test:1"/></head><docTitle><text>Le carnet de lecture</text></docTitle><navMap><navPoint id="a" playOrder="1"><navLabel><text>Le départ</text></navLabel><content src="chapitre1.xhtml"/></navPoint><navPoint id="b" playOrder="2"><navLabel><text>Le retour</text></navLabel><content src="chapitre2.xhtml"/></navPoint></navMap></ncx>''', compress_type=ZIP_DEFLATED)
    for n, heading in ((1, 'Le départ'), (2, 'Le retour')):
        paragraphs = ''.join(f'<p>Paragraphe {i}. À la fenêtre, une lumière douce éclaire le carnet. Éléonore écoute la rivière et prépare son voyage. é è ê à ù ç œ. Le lecteur garde sa place, même lorsque le livre se referme.</p>' for i in range(1, 31))
        z.writestr(f'EPUB/chapitre{n}.xhtml', f'<?xml version="1.0" encoding="UTF-8"?><html xmlns="http://www.w3.org/1999/xhtml"><head><title>{heading}</title></head><body><h1>{heading}</h1><p>Un <strong>mot important</strong> et une <em>pensée légère</em>.<br/>Une nouvelle ligne.</p>{paragraphs}</body></html>', compress_type=ZIP_DEFLATED)
print(target)
