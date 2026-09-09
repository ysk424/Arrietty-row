float2 p=World.xy*.01;
float2 uv=(World.xy-FieldOrigin.xy)/FieldOrigin.z+.5/256.0;
float h=Texture2DSampleLevel(WakeField,WakeFieldSampler,uv,0).r;
float fade=1-smoothstep(2200.0,2800.0,length(World.xy-Boat.xy));
h += .014*sin(dot(p,float2(.96,.28))*1.4-Time*sqrt(9.81*1.4));
h += .009*sin(dot(p,float2(-.4,.9165))*2.6-Time*sqrt(9.81*2.6));
// Mean lake level remains exactly Z=0. Only the dense local patch displaces.
return float3(0,0,h*100*fade*LocalPatch);
