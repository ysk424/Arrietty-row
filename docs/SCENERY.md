# Lake Bled scenery

The source scenery is prepared separately using Secret-World. It is not bundled
in this MIT source repository. Required local input:

`<LakeProject>/Content/Worlds/LakeBled/Maps/LakeBled.umap`

Run `tools/prepare.ps1 -LakeProject <LakeProject>` to copy its native UE assets
into this project's ignored Content directory. The authoring project is read
only. Materials and gameplay are saved to a new `/Game/Row/Maps/BledRow` map.

Baseline: mean water Z=0; geographic origin 46.3636 N, 14.0940 E; mountainous
distance geometry across a roughly 72 km domain. The source contains real meshes
for distant terrain, trees, island and buildings, not a Blender environment
image. SkyAtmosphere, SkyLight, DirectionalLight and fog are UE components.

The local source project's `SourceArt/GeographicSources.json` and export
metadata are the authoritative asset provenance. Keep these notices with any
distributed scenery. Geography uses OpenStreetMap contributors (ODbL 1.0) and
Mapzen terrain tiles derived from EU-DEM / Austria DGM, with original attribution.
MIT applies to the application code, not to these independently licensed data.
Terrain notices retained from the source:

- Produced using Copernicus data and information funded by the European Union - EU-DEM layers.
- © offene Daten Österreichs – Digitales Geländemodell (DGM) Österreich (CC BY 3.0 AT).
- SRTM and GMTED2010 data courtesy of the U.S. Geological Survey.

See [OpenStreetMap copyright](https://www.openstreetmap.org/copyright) and
[Mapzen terrain attribution](https://github.com/tilezen/joerd/blob/master/docs/attribution.md).
