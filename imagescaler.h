#pragma once

/*!
 * this class scales images consisting of 4 components (RGBA)
 * to an arbitrary size using a 4-tap filter
 */
class ImageScaler {
 public:
    struct Filter {
        unsigned  m_offset;
        short     m_coeff[4];
    };

    ImageScaler();
    ~ImageScaler();

    //! Set destination image and source image size
    void SetImageParameters(unsigned *dst_image, unsigned dst_stride, unsigned dst_width, unsigned dst_height,
                            unsigned src_width, unsigned src_height);

    /*! Process one pixel of source image; destination image is written while input is processed
     * SetImageParameters() must be called first
     */
    void PutSourcePixel(unsigned char c0, unsigned char c1, unsigned char c2, unsigned char c3) {
        m_hbuf[ (m_src_x++) & 3 ].Set(c0, c1, c2, c3);

        TmpPixel      *bp = m_buffer + 4 * m_dst_x + (m_src_y & 3);
        const Filter  *fh;

        while ((fh=m_hor_filters + m_dst_x)->m_offset == m_src_x) {
            *bp = m_hbuf[0] * fh->m_coeff[0] + m_hbuf[1] * fh->m_coeff[1] + m_hbuf[2] * fh->m_coeff[2]
                  + m_hbuf[3]*fh->m_coeff[3];
            m_dst_x++;
            bp += 4;
        }

        if (m_src_x == m_src_width) NextSourceLine();
    }

 private:
    //! Temporary image pixel class - a 4-element integer vector
    class TmpPixel {
     public:
        TmpPixel() {
        }

        TmpPixel(int c0, int c1, int c2, int c3) {
            Set(c0, c1, c2, c3);
        }

        void Set(int c0, int c1, int c2, int c3) {
            m_comp[0] = c0;
            m_comp[1] = c1;
            m_comp[2] = c2;
            m_comp[3] = c3;
        }

        TmpPixel operator*(int s) const {
            return TmpPixel(m_comp[0] * s, m_comp[1] * s, m_comp[2] * s, m_comp[3] * s);
        }

        TmpPixel operator+(const TmpPixel &x) const {
            return TmpPixel(m_comp[0] + x[0], m_comp[1] + x[1], m_comp[2] + x[2], m_comp[3] + x[3]);
        }

        // Return component i=[0..3] - No range check!
        int operator[](unsigned i) const {
            return m_comp[i];
        }

     private:
        int m_comp[4];
    };

    //! This is called whenever one input line is processed completely
    void NextSourceLine();

    TmpPixel   m_hbuf[4];      //! Ring buffer for 4 input pixels
    char      *m_memory;       //! Buffer container
    Filter    *m_hor_filters;  //! Buffer for horizontal filters (one for each output image column)
    Filter    *m_ver_filters;  //! Buffer for vertical   filters (one for each output image row)
    TmpPixel  *m_buffer;       //! Buffer contains 4 horizontally filtered input lines, multiplexed
    unsigned  *m_dst_image;    //! Pointer to destination image
    unsigned   m_dst_stride;   //! Destination image stride
    unsigned   m_dst_width;    //! Destination image width
    unsigned   m_dst_height;   //! Destination image height
    unsigned   m_src_width;    //! Source image width
    unsigned   m_src_height;   //! Source image height
    unsigned   m_src_x;        //! x position of next source image pixel
    unsigned   m_src_y;        //! y position of source image line currently beeing processed
    unsigned   m_dst_x;        //! x position of next destination image pixel
    unsigned   m_dst_y;        //! x position of next destination image line
};
