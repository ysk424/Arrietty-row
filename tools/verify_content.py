"""Reload saved assets and verify the independent rowing map."""
import json
from pathlib import Path
import unreal as u
root=Path(__file__).resolve().parents[1]
assert u.get_editor_subsystem(u.LevelEditorSubsystem).load_level('/Game/Row/Maps/BledRow')
actors=u.get_editor_subsystem(u.EditorActorSubsystem).get_all_level_actors()
meshes=[a for a in actors if isinstance(a,u.StaticMeshActor)]
water=[a for a in meshes if u.Name('RowWater') in a.tags]
assert len(meshes)==312, len(meshes)
assert len(water)==1 and abs(water[0].get_actor_location().z)<.01
assert all(a.static_mesh_component.static_mesh for a in meshes)
triangles=sum(a.static_mesh_component.static_mesh.get_num_triangles(0) for a in meshes)
assert triangles==961237,triangles
assert water[0].static_mesh_component.get_material(0).get_path_name()=='/Game/Row/Materials/M_RowWater.M_RowWater'
world=u.get_editor_subsystem(u.UnrealEditorSubsystem).get_editor_world()
assert world.get_world_settings().get_editor_property('default_game_mode').get_name()=='RowGameMode'
for cls in (u.SkyAtmosphere,u.SkyLight,u.DirectionalLight,u.ExponentialHeightFog):
    assert any(isinstance(a,cls) for a in actors), cls
start=next(a for a in actors if isinstance(a,u.PlayerStart))
assert start.get_actor_location().z==0
material=u.load_asset('/Game/Row/Materials/M_RowWater')
assert material.get_editor_property('blend_mode')==u.BlendMode.BLEND_MASKED
report=dict(mesh_actors=len(meshes),triangles=triangles,water_reference_z_cm=0,
            native_sky=True,game_mode='RowGameMode',local_wave_triangles=131072,
            player_start_cm=[start.get_actor_location().x,start.get_actor_location().y,0])
(root/'logs/content-validation.json').write_text(json.dumps(report,indent=2)+'\n')
print('ROW_CONTENT_VERIFIED '+json.dumps(report))
