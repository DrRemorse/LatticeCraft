vertdist = length((matview * position).xyz) / ZFAR;
position.y -= vertdist * vertdist * 32.0;
