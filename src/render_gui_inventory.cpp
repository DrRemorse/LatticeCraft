	/*
namespace cppcraft
{
	void GUIRenderer::renderQuickbarItems(library::mat4& ortho, double frameCounter)
	{
		float width = this->width * 0.4;
		float height = width / 8.0;
		float posx = (this->width - width) * 0.5;
		float posy = this->height - height;
		
		posx += width * 0.025;
		posy += width * 0.025;
		
		float size   = width * 0.075;
		float stride = width * 0.1095;
		
		if (inventory.isChanged())
		{
			// inventory has no longer changed
			inventory.setChanged(false);
			
			// recreate meshes
			quickbarItems.clear();
			
			for (int x = 0; x < inventory.getWidth(); x++)
			{
				Item& itm = inventory(x, menu.quickbarY);
				quickbarItems.emit(itm, posx + x * stride, posy, size);
			}
			quickbarItems.upload();
			
			vec2 vsize(size * 0.25);
			posx += vsize.x * 4;
			posy += vsize.y * 3.25;
			
			// create item numbers
			std::vector<SimpleFont::print_data_t> data;
			SimpleFont::print_data_t printloc;
			
			for (int x = 0; x < inventory.getWidth(); x++)
			{
				Item& itm = inventory(x, menu.quickbarY);
				if (itm.getCount() > 1 && itm.getID() != 0)
				{
					printloc.size     = vsize;
					printloc.text     = std::to_string(itm.getCount());
					printloc.location = vec3(posx + x * stride - printloc.text.length() * vsize.x, posy, 0.0);
					
					data.push_back(printloc);
				}
			}
			quickbarFont.serialUpload(data, false);
		}
		
		// render inventory
		quickbarItems.render(ortho);
		
		// render numbers
		glEnable(GL_BLEND);
		quickbarFont.bind(0);
		quickbarFont.sendMatrix(ortho);
		quickbarFont.setColor(vec4(1.0, 1.0, 1.0, 1.0));
		quickbarFont.setBackColor(vec4(0.0, 0.5));
		
		quickbarFont.render();
		glDisable(GL_BLEND);
	}
}
	*/
