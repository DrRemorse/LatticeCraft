#include "chat.hpp"

#include <library/bitmap/colortools.hpp>
#include <library/opengl/oglfont.hpp>
#include "game.hpp"
#include "renderman.hpp"
#include "shaderman.hpp"
#include <glm/gtx/transform.hpp>
#include <sstream>

using namespace library;
using namespace glm;

namespace cppcraft
{
	Chatbox chatbox;
	static const float CHAT_FADEOUT  = 800.0;
	static const float CHAT_FADETIME = 100.0;
	static const int   CHAT_MAXLINES = 7;

	time_t currentTime()
	{
		time_t rawtime; // long int
		time(&rawtime);
		return rawtime;
	}
	std::string timeString(time_t rawtime)
	{
		tm* t = localtime(&rawtime);
		if (!t) return "[TIME ERROR]";

		std::stringstream ss;
		ss << "[" << t->tm_hour << ":" << t->tm_min << ":" << t->tm_sec << "]";
		return ss.str();
	}

	Chatbox::ChatLine::ChatLine(std::string Source, std::string Text, chattype_t Type)
		: source(Source), text(Text), type(Type), time(currentTime())
	{
		length = timeString(time).size() + 2 + source.length() + text.size();
		if (type == L_CHAT) length += 2;
	}

	void Chatbox::init(float width, float height)
	{
		fadeout   = CHAT_FADEOUT;
		unsigned int col      = BGRA8(0, 0, 0, 144);
		unsigned int col_high = BGRA8(0, 0, 0, 192);

		struct boxvertex_t
		{
			float x, y, z;
			unsigned int color;

			boxvertex_t() {}
			boxvertex_t(float X, float Y, float Z, unsigned int C)
				: x(X), y(Y), z(Z), color(C) {}
		};

		boxvertex_t boxv[4];
		boxv[0] = boxvertex_t(0, 0, 0,  col);
		boxv[1] = boxvertex_t(1, 0, 0,  col);
		boxv[2] = boxvertex_t(1, 1, 0,  col);
		boxv[3] = boxvertex_t(0, 1, 0,  col_high);

		cbvao.begin(sizeof(boxvertex_t), 4, boxv);
		cbvao.attrib(0, 3, GL_FLOAT, GL_FALSE, 0);
		cbvao.attrib(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, offsetof(boxvertex_t, color));
		cbvao.end();
	}
	void Chatbox::openChat(bool open)
	{
		chatOpen = open;
		if (open)
		{
			// reset fading value
			fadeout = CHAT_FADEOUT;
		}
	}
	float Chatbox::getAlpha() const
	{
		if (fadeout < CHAT_FADETIME)
			return fadeout / CHAT_FADETIME;
		return 1.0;
	}


	void Chatbox::add(const std::string& src, const std::string& text, chattype_t type)
	{
		mtx.lock();
		lines.emplace_back(src, text, type);
		if (lines.size() > CHAT_MAXLINES)
		{
			// remove first index
			remove(0);
			// reset fadeout process
			fadeout = CHAT_FADEOUT;
		}
		mtx.unlock();
	}
	void Chatbox::remove(int index)
	{
		auto i = lines.begin()+index;
		lines.erase(i, i+1);
	}

	void Chatbox::renderSourcedMessage(SimpleFont& font, const vec3& spos, const vec2& scale, const std::string& time, const std::string& source, const std::string& text)
	{
		vec3 pos(spos);

		// print time
		font.setColor(glm::vec4(0.5f, 0.5f, 0.5f, getAlpha()));
		font.print(pos, scale, time + " <", false);

		// print source
		pos.x += scale.x * (time.size() + 2);
		font.setColor(glm::vec4(1.0f, 1.0f, 1.0f, getAlpha()));
		font.print(pos, scale, source, false);

		// finish grayed out text
		pos.x += scale.x * source.size();
		font.setColor(glm::vec4(0.5f, 0.5f, 0.5f, getAlpha()));
		font.print(pos, scale, "> ", false);

		// print message
		pos.x += scale.x * 2;
		font.setColor(glm::vec4(1.0f, 1.0f, 1.0f, getAlpha()));
		font.print(pos, scale, text, false);
	}
	void Chatbox::renderInfoMessage(SimpleFont& font, const vec3& spos, const vec2& scale, const std::string& time, const std::string& from, const std::string& text)
	{
		vec3 pos(spos);

		// print time
		font.setColor(glm::vec4(0.5f, 0.5f, 0.5f, getAlpha()));
		font.print(pos, scale, time + " ", false);

		// print from
		pos.x += scale.x * (time.size() + 1);
		font.setColor(glm::vec4(0.0f, 0.0f, 1.0f, getAlpha()));
		font.print(pos, scale, from, false);

		// print message
		pos.x += scale.x * from.size();
		font.setColor(glm::vec4(1.0f, 1.0f, 1.0f, getAlpha()));
		font.print(pos, scale, " " + text, false);
	}
	void Chatbox::renderMessage(SimpleFont& font, const vec3& spos, const vec2& scale, const std::string& time, const std::string& text)
	{
		// print time
		font.setColor(glm::vec4(0.7f, 0.7f, 0.7f, getAlpha()));
		font.print(spos, scale, time + " ", false);

		// print message
		vec3 pos(spos);

		pos.x += scale.x * (time.size() + 1);
		font.setColor(glm::vec4(1.0f, 1.0f, 1.0f, getAlpha()));
		font.print(pos, scale, text, false);
	}
	void Chatbox::bindFont(SimpleFont& font, const glm::mat4& ortho)
	{
		// render chatbox font/text
		font.bind(0);
		font.sendMatrix(ortho);
		font.setBackColor(glm::vec4(0.0f));
		font.setColor(glm::vec4(1.0f, 1.0f, 1.0f, getAlpha()));
	}

	// chatbox main rendering function
	void Chatbox::render(SimpleFont& font, const glm::mat4& ortho, const glm::vec2& textScale, Renderer& renderer)
	{
		float alpha = getAlpha();
		// a hidden box is hidden   -- albert einstein
		if (alpha <= 0.0) return;

		float SH = 1.0 / renderer.aspect();
		vec3 cbPos(0.025, SH * 0.85, 0.0);

		mtx.lock();

		// fade out text over time
		if (this->chatOpen == false)
		{
			this->fadeout -= renderer.delta_time();
			// prevent negative
			if (this->fadeout < 0)
				this->fadeout = 0;
		}
		// clone lines before leaving lock
		std::vector<ChatLine> copy = this->lines;

		mtx.unlock();

		// find longest chatline
		size_t longest = 0;
		for (ChatLine& cl : copy)
		{
			if (longest < cl.length)
				longest = cl.length;
		}

		// minimum background size
		int bgsize = copy.size();

		// chatbox background
		Shader& shd = shaderman[Shaderman::GUI_COLOR];
		shd.bind();
		shd.sendVec4("multcolor", glm::vec4(1.0f, 1.0f, 1.0f, alpha));

		glm::mat4 matbox = ortho;
		matbox *= glm::translate(glm::vec3(cbPos.x, cbPos.y - (bgsize-1) * textScale.y, 0.0f));
		matbox *= glm::scale(glm::vec3(longest * textScale.x, bgsize * textScale.y, 1.0f));
		shd.sendMatrix("mvp", matbox);

		cbvao.render(GL_QUADS);

		// render typing box
		if (chatOpen)
		{
			std::string ctext = game.input().text() + ((((int) renderer.time() / 50) % 2 == 0) ? "_" : " ");
			std::string now = timeString(currentTime());

			const std::string nickname = "**FIXME**";
			size_t msglen = now.size() + 2 + nickname.size() + 2 + ctext.size();

			matbox = ortho;
			matbox *= glm::translate(glm::vec3(cbPos.x, cbPos.y + textScale.y + 0.005f, 0.0f));
			matbox *= glm::scale(glm::vec3(msglen * textScale.x, textScale.y, 1.0f));
			shd.sendMatrix("mvp", matbox);

			cbvao.render(GL_QUADS);

			// render text
			bindFont(font, ortho);

			// print actual text typed into chatbox
			vec3 textPos(cbPos);
			textPos.y += textScale.y + 0.005;
			renderSourcedMessage(font, textPos, textScale, now, nickname, ctext);
		}
		else
		{
			bindFont(font, ortho);
		}

		for (size_t i = 0; i < copy.size(); i++)
		{
			vec3 textPos(cbPos);
			textPos.y -= i * textScale.y;

			ChatLine& cl = copy[copy.size()-1 - i];

			switch (cl.type)
			{
			case Chatbox::L_SERVER:
				renderInfoMessage(font, textPos, textScale, timeString(cl.time), cl.source, cl.text);
				break;
			case Chatbox::L_INFO:
				renderInfoMessage(font, textPos, textScale, timeString(cl.time), cl.source, cl.text);
				break;
			case Chatbox::L_CHAT:
				renderInfoMessage(font, textPos, textScale, timeString(cl.time), cl.source, cl.text);
				break;
			}
		}

	}
}
