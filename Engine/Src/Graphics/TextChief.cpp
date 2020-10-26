#include "TextChief.h"

///Can be improved

extern int winWidth;
extern int winHeight;

TextChief::TextChief():
    ft(),
    face(),
    allChars({}),
    VAO(0),
    VBO(0)
{
}

TextChief::~TextChief(){
    FT_Done_Face(face);
    FT_Done_FreeType(ft);
}

bool TextChief::Init(){
    if(FT_Init_FreeType(&ft)){
        (void)puts("Failed to init FreeType!\n");
        return false;
    }
    if(FT_New_Face(ft, "Fonts/Yes.ttf", 0, &face)){
        (void)puts("Failed to load font as face\n");
        return false;
    }
    FT_Set_Pixel_Sizes(face, 0, 48); //Define pixel font size to extract from font face //0 for w so dynamically calc w based on h
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); //Disable byte-alignment restriction

    for(unsigned char c = 0; c < 128; ++c){ //Load 1st 128 ASCII chars
        if(FT_Load_Char(face, c, FT_LOAD_RENDER)){ //Load char glyph and set it as active
            (void)printf("Failed to load \'%c\' as char glyph\n", char(c));
            continue;
        }

        uint texRefID;
        glGenTextures(1, &texRefID);
        glBindTexture(GL_TEXTURE_2D, texRefID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, face->glyph->bitmap.width, face->glyph->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        allChars.insert(std::pair<char, CharMetrics>(c, {
            texRefID,
            uint(face->glyph->advance.x),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
        }));

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0); 
    }

    return true;
}

void TextChief::RenderText(ShaderProg& SP, const TextAttribs& attribs){
    if(!ft && !Init()){
        puts("Failed to init TextChief!\n");
    }

    SP.Use();
    SP.Set4fv("textColour", attribs.colour);
    SP.UseTex(attribs.texRefID, "textTex");
    SP.SetMat4fv("projection", &glm::ortho(0.0f, (float)winWidth, 0.0f, (float)winHeight)[0][0]);

    glBindVertexArray(VAO);
    for(std::string::const_iterator c = attribs.text.begin(); c != attribs.text.end(); ++c){
        CharMetrics ch = allChars[*c];
        float xpos = attribs.x + ch.bearing.x * attribs.scaleFactor;
        float ypos = attribs.y - (ch.size.y - ch.bearing.y) * attribs.scaleFactor;

        float w = ch.size.x * attribs.scaleFactor;
        float h = ch.size.y * attribs.scaleFactor;

        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 0.0f },            
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f },

            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f }           
        };

        SP.UseTex(ch.texRefID, "texSampler");

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); 
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glDrawArrays(GL_TRIANGLES, 0, 6);

        const_cast<float&>(attribs.x) += (ch.advance >> 6) * attribs.scaleFactor; // now advance cursors for next glyph (note that advance is number of 1/64 pixels) // bitshift by 6 to get value in pixels (2^6 = 64)
    }

    SP.ResetTexUnits();
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
}