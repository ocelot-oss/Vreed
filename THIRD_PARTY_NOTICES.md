Vreed is distributed under the GNU Affero General Public License version 3 or later; see LICENSE.

The PDF renderer is MuPDF 1.28.5, copyright Artifex Software, AGPL-3.0-or-later, pinned to commit 8ad45e92f0935d3d87f1db3f873086472a5e1b24 at https://github.com/ArtifexSoftware/mupdf. tools/build_mupdf.sh retrieves the corresponding source and its pinned dependencies and builds it for Vita or Linux. Their license notices remain in their source directories.

The patched font renderer in third_party/libvita2d-font is copyright Sergi Granell, MIT licensed; its original license is included there. DejaVu fonts retain their license in assets/fonts/LICENSE.

Complete corresponding source archives are published alongside each VPK in GitHub Actions as Vreed-Sources. The Vreed source and build recipes are also available at https://github.com/ocelot-oss/Vreed.
