/**
 * Chatbox showing you unimportant text
 * 
**/
#ifndef CHAT_HPP
#define CHAT_HPP

#include <library/opengl/vao.hpp>
#include <mutex>
#include <string>
#include <time.h>
#include <vector>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

namespace library
{
	class SimpleFont;
}

namespace cppcraft
{
	class Renderer;
	extern time_t currentTime();
	
	class Chatbox
	{
	public:
		enum chattype_t
		{
			L_INFO,
			L_SERVER,
			L_CHAT
		};
		
		struct ChatLine
		{
			std::string source;
			std::string text;
			chattype_t  type;
			time_t time;
			size_t length;
			bool   newline;
			
			ChatLine(std::string Source, std::string Text, chattype_t Type);
		};
		
		typedef unsigned int color_t;
		
		void init(float width, float height);
		void render(library::SimpleFont& font, const glm::mat4& ortho, const glm::vec2& textScale, Renderer& renderer);
		void add(const std::string& source, const std::string& text, chattype_t type);
		
		void openChat(bool open);
		inline bool isOpen() const { return chatOpen; }
		
		// chat fade/alpha value
		float getAlpha() const;
		
	private:
		float  fadeout;
		library::VAO cbvao;
		
		std::vector<ChatLine> lines;
		std::mutex  mtx;
		bool chatOpen;
		
		void remove(int index);
		
		void bindFont(library::SimpleFont& font, const glm::mat4& ortho);
		
		void renderSourcedMessage
		(
			library::SimpleFont& font, 
			const glm::vec3& spos, 
			const glm::vec2& scale, 
			const std::string& time, 
			const std::string& source, 
			const std::string& text
		);
		void renderInfoMessage
		(
			library::SimpleFont& font, 
			const glm::vec3& spos, 
			const glm::vec2& scale, 
			const std::string& time, 
			const std::string& from, 
			const std::string& text
		);
		void renderMessage
		(
			library::SimpleFont& font, 
			const glm::vec3& spos, 
			const glm::vec2& scale, 
			const std::string& time, 
			const std::string& text
		);
	};
	extern Chatbox chatbox;
}

#endif
