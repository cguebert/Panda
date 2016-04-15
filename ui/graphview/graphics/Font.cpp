#include <ui/graphview/graphics/Font.h>
#include <ui/graphview/graphics/DrawList.h>

#include <algorithm>
#include <cassert>

namespace
{
	
// Convert UTF-8 to 32-bits character, process single character input.
// Based on stb_from_utf8() from github.com/nothings/stb/
// We handle UTF-8 decoding error by skipping forward.
int TextCharFromUtf8(unsigned int* out_char, const char* in_text, const char* in_text_end)
{
    unsigned int c = (unsigned int)-1;
	const unsigned char* str = (const unsigned char*)in_text;
    if (!(*str & 0x80))
    {
        c = (unsigned int)(*str++);
        *out_char = c;
        return 1;
    }
    if ((*str & 0xe0) == 0xc0)
    {
        *out_char = 0xFFFD; // will be invalid but not end of string
        if (in_text_end && in_text_end - (const char*)str < 2) return 1;
        if (*str < 0xc2) return 2;
        c = (unsigned int)((*str++ & 0x1f) << 6);
        if ((*str & 0xc0) != 0x80) return 2;
        c += (*str++ & 0x3f);
        *out_char = c;
        return 2;
    }
    if ((*str & 0xf0) == 0xe0)
    {
        *out_char = 0xFFFD; // will be invalid but not end of string
        if (in_text_end && in_text_end - (const char*)str < 3) return 1;
        if (*str == 0xe0 && (str[1] < 0xa0 || str[1] > 0xbf)) return 3;
        if (*str == 0xed && str[1] > 0x9f) return 3; // str[1] < 0x80 is checked below
        c = (unsigned int)((*str++ & 0x0f) << 12);
        if ((*str & 0xc0) != 0x80) return 3;
        c += (unsigned int)((*str++ & 0x3f) << 6);
        if ((*str & 0xc0) != 0x80) return 3;
        c += (*str++ & 0x3f);
        *out_char = c;
        return 3;
    }
    if ((*str & 0xf8) == 0xf0)
    {
        *out_char = 0xFFFD; // will be invalid but not end of string
        if (in_text_end && in_text_end - (const char*)str < 4) return 1;
        if (*str > 0xf4) return 4;
        if (*str == 0xf0 && (str[1] < 0x90 || str[1] > 0xbf)) return 4;
        if (*str == 0xf4 && str[1] > 0x8f) return 4; // str[1] < 0x80 is checked below
        c = (unsigned int)((*str++ & 0x07) << 18);
        if ((*str & 0xc0) != 0x80) return 4;
        c += (unsigned int)((*str++ & 0x3f) << 12);
        if ((*str & 0xc0) != 0x80) return 4;
        c += (unsigned int)((*str++ & 0x3f) << 6);
        if ((*str & 0xc0) != 0x80) return 4;
        c += (*str++ & 0x3f);
        // utf-8 encodings of values used in surrogate pairs are invalid
        if ((c & 0xFFFFF800) == 0xD800) return 4;
        *out_char = c;
        return 4;
    }
    *out_char = 0;
    return 0;
}

int TextStrFromUtf8(WChar* buf, int buf_size, const char* in_text, const char* in_text_end, const char** in_text_remaining)
{
    WChar* buf_out = buf;
    WChar* buf_end = buf + buf_size;
    while (buf_out < buf_end-1 && (!in_text_end || in_text < in_text_end) && *in_text)
    {
        unsigned int c;
        in_text += TextCharFromUtf8(&c, in_text, in_text_end);
        if (c == 0)
            break;
        if (c < 0x10000)    // FIXME: Losing characters that don't fit in 2 bytes
            *buf_out++ = (WChar)c;
    }
    *buf_out = 0;
    if (in_text_remaining)
        *in_text_remaining = in_text;
    return (int)(buf_out - buf);
}

int TextCountCharsFromUtf8(const char* in_text, const char* in_text_end)
{
    int char_count = 0;
    while ((!in_text_end || in_text < in_text_end) && *in_text)
    {
        unsigned int c;
        in_text += TextCharFromUtf8(&c, in_text, in_text_end);
        if (c == 0)
            break;
        if (c < 0x10000)
            char_count++;
    }
    return char_count;
}

// Based on stb_to_utf8() from github.com/nothings/stb/
static inline int TextCharToUtf8(char* buf, int buf_size, unsigned int c)
{
    if (c < 0x80)
    {
        buf[0] = (char)c;
        return 1;
    }
    if (c < 0x800)
    {
        if (buf_size < 2) return 0;
        buf[0] = (char)(0xc0 + (c >> 6));
        buf[1] = (char)(0x80 + (c & 0x3f));
        return 2;
    }
    if (c >= 0xdc00 && c < 0xe000)
    {
        return 0;
    }
    if (c >= 0xd800 && c < 0xdc00)
    {
        if (buf_size < 4) return 0;
        buf[0] = (char)(0xf0 + (c >> 18));
        buf[1] = (char)(0x80 + ((c >> 12) & 0x3f));
        buf[2] = (char)(0x80 + ((c >> 6) & 0x3f));
        buf[3] = (char)(0x80 + ((c ) & 0x3f));
        return 4;
    }
    //else if (c < 0x10000)
    {
        if (buf_size < 3) return 0;
        buf[0] = (char)(0xe0 + (c >> 12));
        buf[1] = (char)(0x80 + ((c>> 6) & 0x3f));
        buf[2] = (char)(0x80 + ((c ) & 0x3f));
        return 3;
    }
}

static inline int TextCountUtf8BytesFromChar(unsigned int c)
{
    if (c < 0x80) return 1;
    if (c < 0x800) return 2;
    if (c >= 0xdc00 && c < 0xe000) return 0;
    if (c >= 0xd800 && c < 0xdc00) return 4;
    return 3;
}

int TextStrToUtf8(char* buf, int buf_size, const WChar* in_text, const WChar* in_text_end)
{
    char* buf_out = buf;
    const char* buf_end = buf + buf_size;
    while (buf_out < buf_end-1 && (!in_text_end || in_text < in_text_end) && *in_text)
    {
        unsigned int c = (unsigned int)(*in_text++);
        if (c < 0x80)
            *buf_out++ = (char)c;
        else
            buf_out += TextCharToUtf8(buf_out, (int)(buf_end-buf_out-1), c);
    }
    *buf_out = 0;
    return (int)(buf_out - buf);
}

int TextCountUtf8BytesFromStr(const WChar* in_text, const WChar* in_text_end)
{
    int bytes_count = 0;
    while ((!in_text_end || in_text < in_text_end) && *in_text)
    {
        unsigned int c = (unsigned int)(*in_text++);
        if (c < 0x80)
            bytes_count++;
        else
            bytes_count += TextCountUtf8BytesFromChar(c);
    }
    return bytes_count;
}

static inline bool CharIsSpace(int c) { return c == ' ' || c == '\t' || c == 0x3000; }


} // Unammed namespace

//****************************************************************************//

Font::Font()
{
	clear();
}

Font::~Font()
{
    clear();
}

void Font::clear()
{
    m_fontSize = 0.0f;
	m_displayOffset = { 0.0f, 1.0f };
    m_configData = NULL;
    m_configDataCount = 0;
    m_ascent = m_descent = 0.0f;
    m_containerAtlas = NULL;
    m_glyphs.clear();
    m_fallbackGlyph = NULL;
    m_fallbackXAdvance = 0.0f;
    m_indexXAdvance.clear();
    m_indexLookup.clear();
}

void Font::buildLookupTable()
{
    int max_codepoint = 0;
    for (int i = 0; i != m_glyphs.size(); i++)
        max_codepoint = std::max(max_codepoint, (int)m_glyphs[i].Codepoint);

    m_indexXAdvance.clear();
    m_indexXAdvance.resize(max_codepoint + 1);
    m_indexLookup.clear();
    m_indexLookup.resize(max_codepoint + 1);
    for (int i = 0; i < max_codepoint + 1; i++)
    {
        m_indexXAdvance[i] = -1.0f;
        m_indexLookup[i] = -1;
    }
    for (size_t i = 0; i < m_glyphs.size(); i++)
    {
        int codepoint = (int)m_glyphs[i].Codepoint;
        m_indexXAdvance[codepoint] = m_glyphs[i].XAdvance;
        m_indexLookup[codepoint] = i;
    }

    // Create a glyph to handle TAB
    // FIXME: Needs proper TAB handling but it needs to be contextualized (or we could arbitrary say that each string starts at "column 0" ?)
    if (findGlyph((unsigned short)' '))
    {
        if (m_glyphs.back().Codepoint != '\t')   // So we can call this function multiple times
            m_glyphs.resize(m_glyphs.size() + 1);
        Font::Glyph& tab_glyph = m_glyphs.back();
        tab_glyph = *findGlyph((unsigned short)' ');
        tab_glyph.Codepoint = '\t';
        tab_glyph.XAdvance *= 4;
        m_indexXAdvance[(int)tab_glyph.Codepoint] = (float)tab_glyph.XAdvance;
        m_indexLookup[(int)tab_glyph.Codepoint] = (int)(m_glyphs.size()-1);
    }

    m_fallbackGlyph = NULL;
    m_fallbackGlyph = findGlyph(m_fallbackChar);
    m_fallbackXAdvance = m_fallbackGlyph ? m_fallbackGlyph->XAdvance : 0.0f;
    for (int i = 0; i < max_codepoint + 1; i++)
        if (m_indexXAdvance[i] < 0.0f)
            m_indexXAdvance[i] = m_fallbackXAdvance;
}

void Font::setFallbackChar(WChar c)
{
    m_fallbackChar = c;
    buildLookupTable();
}

const Font::Glyph* Font::findGlyph(unsigned short c) const
{
    if (c < m_indexLookup.size())
    {
        const int i = m_indexLookup[c];
        if (i != -1)
            return &m_glyphs[i];
    }
    return m_fallbackGlyph;
}

const char* Font::calcWordWrapPositionA(float scale, const char* text, const char* text_end, float wrap_width) const
{
    // Simple word-wrapping for English, not full-featured. Please submit failing cases!
    // FIXME: Much possible improvements (don't cut things like "word !", "word!!!" but cut within "word,,,,", more sensible support for punctuations, support for Unicode punctuations, etc.)

    // For references, possible wrap point marked with ^
    //  "aaa bbb, ccc,ddd. eee   fff. ggg!"
    //      ^    ^    ^   ^   ^__    ^    ^

    // List of hardcoded separators: .,;!?'"

    // Skip extra blanks after a line returns (that includes not counting them in width computation)
    // e.g. "Hello    world" --> "Hello" "World"

    // Cut words that cannot possibly fit within one line.
    // e.g.: "The tropical fish" with ~5 characters worth of width --> "The tr" "opical" "fish"

    float line_width = 0.0f;
    float word_width = 0.0f;
    float blank_width = 0.0f;

    const char* word_end = text;
    const char* prev_word_end = NULL;
    bool inside_word = true;

    const char* s = text;
    while (s < text_end)
    {
        unsigned int c = (unsigned int)*s;
        const char* next_s;
        if (c < 0x80)
            next_s = s + 1;
        else
            next_s = s + TextCharFromUtf8(&c, s, text_end);
        if (c == 0)
            break;

        if (c < 32)
        {
            if (c == '\n')
            {
                line_width = word_width = blank_width = 0.0f;
                inside_word = true;
                s = next_s;
                continue;
            }
            if (c == '\r')
            {
                s = next_s;
                continue;
            }
        }

        const float char_width = ((int)c < m_indexXAdvance.size()) ? m_indexXAdvance[(int)c] * scale : m_fallbackXAdvance;
        if (CharIsSpace(c))
        {
            if (inside_word)
            {
                line_width += blank_width;
                blank_width = 0.0f;
            }
            blank_width += char_width;
            inside_word = false;
        }
        else
        {
            word_width += char_width;
            if (inside_word)
            {
                word_end = next_s;
            }
            else
            {
                prev_word_end = word_end;
                line_width += word_width + blank_width;
                word_width = blank_width = 0.0f;
            }

            // Allow wrapping after punctuation.
            inside_word = !(c == '.' || c == ',' || c == ';' || c == '!' || c == '?' || c == '\"');
        }

        // We ignore blank width at the end of the line (they can be skipped)
        if (line_width + word_width >= wrap_width)
        {
            // Words that cannot possibly fit within an entire line will be cut anywhere.
            if (word_width < wrap_width)
                s = prev_word_end ? prev_word_end : word_end;
            break;
        }

        s = next_s;
    }

    return s;
}

pPoint Font::calcTextSizeA(float size, float max_width, float wrap_width, const char* text_begin, const char* text_end, const char** remaining) const
{
    if (!text_end)
        text_end = text_begin + strlen(text_begin); // FIXME-OPT: Need to avoid this.

    const float line_height = size;
    const float scale = size / m_fontSize;

    pPoint text_size = pPoint(0,0);
    float line_width = 0.0f;

    const bool word_wrap_enabled = (wrap_width > 0.0f);
    const char* word_wrap_eol = NULL;

    const char* s = text_begin;
    while (s < text_end)
    {
        if (word_wrap_enabled)
        {
            // Calculate how far we can render. Requires two passes on the string data but keeps the code simple and not intrusive for what's essentially an uncommon feature.
            if (!word_wrap_eol)
            {
                word_wrap_eol = calcWordWrapPositionA(scale, s, text_end, wrap_width - line_width);
                if (word_wrap_eol == s) // Wrap_width is too small to fit anything. Force displaying 1 character to minimize the height discontinuity.
                    word_wrap_eol++;    // +1 may not be a character start point in UTF-8 but it's ok because we use s >= word_wrap_eol below
            }

            if (s >= word_wrap_eol)
            {
                if (text_size.x < line_width)
                    text_size.x = line_width;
                text_size.y += line_height;
                line_width = 0.0f;
                word_wrap_eol = NULL;

                // Wrapping skips upcoming blanks
                while (s < text_end)
                {
                    const char c = *s;
                    if (CharIsSpace(c)) { s++; } else if (c == '\n') { s++; break; } else { break; }
                }
                continue;
            }
        }

        // Decode and advance source
        const char* prev_s = s;
        unsigned int c = (unsigned int)*s;
        if (c < 0x80)
        {
            s += 1;
        }
        else
        {
            s += TextCharFromUtf8(&c, s, text_end);
            if (c == 0)
                break;
        }

        if (c < 32)
        {
            if (c == '\n')
            {
                text_size.x = std::max(text_size.x, line_width);
                text_size.y += line_height;
                line_width = 0.0f;
                continue;
            }
            if (c == '\r')
                continue;
        }

        const float char_width = ((int)c < m_indexXAdvance.size() ? m_indexXAdvance[(int)c] : m_fallbackXAdvance) * scale;
        if (line_width + char_width >= max_width)
        {
            s = prev_s;
            break;
        }

        line_width += char_width;
    }

    if (text_size.x < line_width)
        text_size.x = line_width;

    if (line_width > 0 || text_size.y == 0.0f)
        text_size.y += line_height;

    if (remaining)
        *remaining = s;

    return text_size;
}

void Font::renderText(float size, pPoint pos, unsigned int col, const pRect& clip_rect,
					  const char* text_begin, const char* text_end, DrawList* draw_list,
					  float wrap_width, bool cpu_fine_clip) const
{
    if (!text_end)
        text_end = text_begin + strlen(text_begin);

    // Align to be pixel perfect
    pos.x = (float)(int)pos.x + m_displayOffset.x;
    pos.y = (float)(int)pos.y + m_displayOffset.y;
    float x = pos.x;
    float y = pos.y;
    if (y > clip_rect.bottom())
        return;

    const float scale = size / m_fontSize;
    const float line_height = m_fontSize * scale;
    const bool word_wrap_enabled = (wrap_width > 0.0f);
    const char* word_wrap_eol = NULL;

    DrawVert* vtx_write = draw_list->vtxWritePtr();
    DrawIdx* idx_write = draw_list->idxWritePtr();
    DrawIdx vtx_current_idx = draw_list->vtxCurrentIdx();

    const char* s = text_begin;
    if (!word_wrap_enabled && y + line_height < clip_rect.top())
        while (s < text_end && *s != '\n')  // Fast-forward to next line
            s++;
    while (s < text_end)
    {
        if (word_wrap_enabled)
        {
            // Calculate how far we can render. Requires two passes on the string data but keeps the code simple and not intrusive for what's essentially an uncommon feature.
            if (!word_wrap_eol)
            {
                word_wrap_eol = calcWordWrapPositionA(scale, s, text_end, wrap_width - (x - pos.x));
                if (word_wrap_eol == s) // Wrap_width is too small to fit anything. Force displaying 1 character to minimize the height discontinuity.
                    word_wrap_eol++;    // +1 may not be a character start point in UTF-8 but it's ok because we use s >= word_wrap_eol below
            }

            if (s >= word_wrap_eol)
            {
                x = pos.x;
                y += line_height;
                word_wrap_eol = NULL;

                // Wrapping skips upcoming blanks
                while (s < text_end)
                {
                    const char c = *s;
                    if (CharIsSpace(c)) { s++; } else if (c == '\n') { s++; break; } else { break; }
                }
                continue;
            }
        }

        // Decode and advance source
        unsigned int c = (unsigned int)*s;
        if (c < 0x80)
        {
            s += 1;
        }
        else
        {
            s += TextCharFromUtf8(&c, s, text_end);
            if (c == 0)
                break;
        }

        if (c < 32)
        {
            if (c == '\n')
            {
                x = pos.x;
                y += line_height;

                if (y > clip_rect.bottom())
                    break;
                if (!word_wrap_enabled && y + line_height < clip_rect.top())
                    while (s < text_end && *s != '\n')  // Fast-forward to next line
                        s++;
                continue;
            }
            if (c == '\r')
                continue;
        }

        float char_width = 0.0f;
        if (const Glyph* glyph = findGlyph((unsigned short)c))
        {
            char_width = glyph->XAdvance * scale;

            // Arbitrarily assume that both space and tabs are empty glyphs as an optimization
            if (c != ' ' && c != '\t')
            {
                // We don't do a second finer clipping test on the Y axis as we've already skipped anything before clip_rect.y and exit once we pass clip_rect.w
                float y1 = (float)(y + glyph->Y0 * scale);
                float y2 = (float)(y + glyph->Y1 * scale);

                float x1 = (float)(x + glyph->X0 * scale);
                float x2 = (float)(x + glyph->X1 * scale);
                if (x1 <= clip_rect.right() && x2 >= clip_rect.left())
                {
                    // Render a character
                    float u1 = glyph->U0;
                    float v1 = glyph->V0;
                    float u2 = glyph->U1;
                    float v2 = glyph->V1;

                    // CPU side clipping used to fit text in their frame when the frame is too small. Only does clipping for axis aligned quads.
                    if (cpu_fine_clip)
                    {
                        if (x1 < clip_rect.left())
                        {
                            u1 = u1 + (1.0f - (x2 - clip_rect.left()) / (x2 - x1)) * (u2 - u1);
                            x1 = clip_rect.left();
                        }
                        if (y1 < clip_rect.top())
                        {
                            v1 = v1 + (1.0f - (y2 - clip_rect.top()) / (y2 - y1)) * (v2 - v1);
                            y1 = clip_rect.top();
                        }
                        if (x2 > clip_rect.right())
                        {
                            u2 = u1 + ((clip_rect.right() - x1) / (x2 - x1)) * (u2 - u1);
                            x2 = clip_rect.right();
                        }
                        if (y2 > clip_rect.bottom())
                        {
                            v2 = v1 + ((clip_rect.bottom() - y1) / (y2 - y1)) * (v2 - v1);
                            y2 = clip_rect.bottom();
                        }
                        if (y1 >= y2)
                        {
                            x += char_width;
                            continue;
                        }
                    }

                    // We are NOT calling PrimRectUV() here because non-inlined causes too much overhead in a debug build.
                    // Inlined here:
                    {
                        idx_write[0] = vtx_current_idx; idx_write[1] = vtx_current_idx+1; idx_write[2] = vtx_current_idx+2;
                        idx_write[3] = vtx_current_idx; idx_write[4] = vtx_current_idx+2; idx_write[5] = vtx_current_idx+3;
                        vtx_write[0].pos.x = x1; vtx_write[0].pos.y = y1; vtx_write[0].col = col; vtx_write[0].uv.x = u1; vtx_write[0].uv.y = v1;
                        vtx_write[1].pos.x = x2; vtx_write[1].pos.y = y1; vtx_write[1].col = col; vtx_write[1].uv.x = u2; vtx_write[1].uv.y = v1;
                        vtx_write[2].pos.x = x2; vtx_write[2].pos.y = y2; vtx_write[2].col = col; vtx_write[2].uv.x = u2; vtx_write[2].uv.y = v2;
                        vtx_write[3].pos.x = x1; vtx_write[3].pos.y = y2; vtx_write[3].col = col; vtx_write[3].uv.x = u1; vtx_write[3].uv.y = v2;
                        vtx_write += 4;
                        vtx_current_idx += 4;
                        idx_write += 6;
                    }
                }
            }
        }

        x += char_width;
    }

    draw_list->setVtxWritePtr(vtx_write);
    draw_list->setVtxCurrentIdx(vtx_current_idx);
    draw_list->setIdxWritePtr(idx_write);
}