static unsigned char png_search_png[] = {
  0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d,
  0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x20,
  0x08, 0x06, 0x00, 0x00, 0x00, 0x73, 0x7a, 0x7a, 0xf4, 0x00, 0x00, 0x00,
  0x09, 0x70, 0x48, 0x59, 0x73, 0x00, 0x00, 0x13, 0xaf, 0x00, 0x00, 0x13,
  0xaf, 0x01, 0x63, 0xe6, 0x8e, 0xc3, 0x00, 0x00, 0x00, 0x19, 0x74, 0x45,
  0x58, 0x74, 0x53, 0x6f, 0x66, 0x74, 0x77, 0x61, 0x72, 0x65, 0x00, 0x77,
  0x77, 0x77, 0x2e, 0x69, 0x6e, 0x6b, 0x73, 0x63, 0x61, 0x70, 0x65, 0x2e,
  0x6f, 0x72, 0x67, 0x9b, 0xee, 0x3c, 0x1a, 0x00, 0x00, 0x02, 0x50, 0x49,
  0x44, 0x41, 0x54, 0x58, 0x85, 0xcd, 0xd7, 0xcb, 0xab, 0x8d, 0x61, 0x14,
  0xc7, 0xf1, 0xcf, 0xe3, 0x48, 0x67, 0x44, 0xb9, 0x1c, 0x4e, 0x2e, 0x25,
  0x0c, 0x8d, 0x90, 0x21, 0x11, 0x99, 0x61, 0xe2, 0x32, 0x35, 0xf0, 0x37,
  0x18, 0xc8, 0xcc, 0x25, 0x43, 0x33, 0x23, 0x19, 0x30, 0x32, 0xa1, 0x18,
  0xb8, 0x96, 0x92, 0x88, 0x12, 0x85, 0x08, 0x03, 0xb7, 0x1c, 0x72, 0x1b,
  0x88, 0x3a, 0x2c, 0x83, 0x77, 0x6d, 0xb6, 0xdd, 0x39, 0x67, 0xbf, 0xef,
  0xb6, 0x73, 0x3c, 0xf5, 0xb4, 0x76, 0x7b, 0x3f, 0xeb, 0xb7, 0xbe, 0xcf,
  0x7a, 0x9f, 0xf5, 0xac, 0xfd, 0x96, 0x88, 0x30, 0x99, 0x63, 0xca, 0xa4,
  0x46, 0xff, 0x1f, 0x00, 0xa6, 0xd6, 0x5d, 0x58, 0x4a, 0x59, 0x81, 0x2d,
  0x58, 0x87, 0x45, 0x18, 0xc2, 0x07, 0x8c, 0xe0, 0x26, 0xce, 0xe0, 0x7c,
  0x44, 0x7c, 0x6d, 0x44, 0x10, 0x11, 0x13, 0x4e, 0xac, 0xc2, 0x65, 0x44,
  0x8d, 0xf9, 0x02, 0xbb, 0x31, 0xb5, 0x9b, 0xee, 0x2f, 0xfd, 0x09, 0x02,
  0x17, 0xec, 0xc5, 0x8f, 0x14, 0x1f, 0xc1, 0x11, 0x6c, 0xc4, 0x02, 0x4c,
  0xc3, 0x5c, 0xac, 0xc0, 0x1e, 0xdc, 0x69, 0x03, 0xb9, 0x80, 0x99, 0x3d,
  0x03, 0x64, 0xf0, 0x63, 0x29, 0x36, 0x8a, 0xfd, 0x98, 0xde, 0x25, 0x53,
  0x05, 0xdb, 0xf0, 0x32, 0xfd, 0x1e, 0x61, 0x5e, 0xaf, 0x00, 0x7b, 0x53,
  0xe4, 0x13, 0x36, 0xd5, 0x4d, 0x67, 0xfa, 0xce, 0xc7, 0xad, 0xf4, 0xbf,
  0x8e, 0xc1, 0x46, 0x00, 0x58, 0x9d, 0x69, 0x1f, 0x6d, 0x1a, 0xbc, 0x4d,
  0x63, 0x36, 0x9e, 0x24, 0xc4, 0xc1, 0xa6, 0x00, 0x57, 0xd2, 0xf1, 0x40,
  0x2f, 0xc1, 0xdb, 0x74, 0x56, 0xe2, 0x3b, 0xbe, 0x60, 0x41, 0x2d, 0x80,
  0x3c, 0x50, 0x81, 0xb7, 0xdd, 0x9e, 0x79, 0x4d, 0x88, 0x13, 0xdd, 0xb2,
  0xd0, 0x79, 0x11, 0x6d, 0x4d, 0x7b, 0x32, 0x22, 0x3e, 0x8f, 0x5f, 0xbc,
  0xb5, 0xc7, 0xd1, 0xb4, 0x9b, 0xc7, 0x5d, 0xd1, 0x41, 0x7c, 0x2d, 0x89,
  0x37, 0xfc, 0xed, 0xee, 0x53, 0x6f, 0x00, 0xef, 0x53, 0x73, 0xb8, 0x4e,
  0x06, 0x16, 0xa6, 0x7d, 0xd8, 0x87, 0xdd, 0x8b, 0x88, 0xef, 0x78, 0xdc,
  0xa1, 0xfd, 0xc7, 0xe8, 0x04, 0x18, 0x4a, 0x3b, 0xd2, 0x0f, 0x80, 0x1c,
  0x6f, 0xd2, 0xce, 0xad, 0x03, 0xd0, 0x7a, 0xee, 0xd3, 0xfb, 0x08, 0x30,
  0x23, 0xed, 0xa7, 0x3a, 0x00, 0xaf, 0xd2, 0x2e, 0xea, 0x23, 0x40, 0x2b,
  0xf5, 0xaf, 0xeb, 0x00, 0xdc, 0x4e, 0xbb, 0xa1, 0x1f, 0x91, 0x4b, 0x29,
  0x4b, 0xb1, 0x58, 0x75, 0x10, 0x9f, 0xd5, 0x01, 0x38, 0x9d, 0x76, 0x47,
  0x29, 0xa5, 0xf4, 0x81, 0x61, 0x67, 0xda, 0xb3, 0x11, 0x31, 0x3a, 0xe6,
  0x8a, 0x8e, 0xb2, 0x19, 0xc4, 0x73, 0x55, 0xd9, 0x6c, 0xff, 0xcb, 0x12,
  0x9c, 0xa5, 0xfa, 0xbf, 0x10, 0x58, 0xdb, 0xe4, 0x2a, 0xde, 0x95, 0x4e,
  0xaf, 0x30, 0xbf, 0xc7, 0xe0, 0x05, 0xa7, 0x52, 0xe7, 0x6c, 0xd3, 0x5e,
  0x30, 0x80, 0x8b, 0xe9, 0x7c, 0x0b, 0xb3, 0x7b, 0x08, 0x7e, 0x38, 0xfd,
  0x3f, 0x60, 0x49, 0x23, 0x80, 0x14, 0x99, 0xa9, 0xea, 0xe7, 0xa1, 0xea,
  0x6a, 0x2b, 0x1b, 0xa4, 0xbd, 0xb5, 0xf3, 0x50, 0x1d, 0xbc, 0xa1, 0xc6,
  0x00, 0x29, 0x36, 0xac, 0xea, 0xe7, 0xa1, 0x6a, 0xcf, 0x27, 0xb0, 0x06,
  0x03, 0x63, 0xac, 0x5d, 0x86, 0x7d, 0xf8, 0xd8, 0xb6, 0xf3, 0x67, 0xf9,
  0xf9, 0xde, 0x44, 0x10, 0xdd, 0x76, 0x34, 0x88, 0x43, 0xaa, 0x96, 0x1a,
  0x6d, 0xe2, 0x37, 0x54, 0x15, 0x73, 0xb5, 0x2d, 0x50, 0x6b, 0x9e, 0xc3,
  0x52, 0xcc, 0xc1, 0xdd, 0xfc, 0xee, 0x81, 0x71, 0x7a, 0x41, 0xdd, 0xe7,
  0xba, 0x10, 0x07, 0x71, 0xbf, 0x23, 0x58, 0x6b, 0xbe, 0xc3, 0xf1, 0xce,
  0xd3, 0x5e, 0x07, 0xa2, 0xe4, 0xc2, 0xda, 0xa3, 0x94, 0x32, 0xac, 0xba,
  0x29, 0xe7, 0x65, 0x36, 0x5e, 0xe3, 0x69, 0x54, 0x8d, 0x67, 0xac, 0xf5,
  0x73, 0x70, 0x09, 0xcb, 0x55, 0x4d, 0x6e, 0x5d, 0x44, 0xfc, 0xbe, 0x15,
  0x7b, 0x29, 0xb3, 0x1e, 0xca, 0x72, 0x28, 0xcf, 0x42, 0xe0, 0x66, 0xfb,
  0x6f, 0xff, 0xe4, 0xcd, 0x28, 0x22, 0x46, 0xb0, 0x5e, 0x75, 0xd5, 0x7f,
  0xfb, 0x23, 0x43, 0x4d, 0x1f, 0x41, 0xbf, 0xc7, 0xa4, 0xbf, 0x1b, 0x4e,
  0x3a, 0xc0, 0x4f, 0xb1, 0xf3, 0x9f, 0x19, 0xa3, 0x3c, 0xc2, 0xfe, 0x00,
  0x00, 0x00, 0x00, 0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82
};
static unsigned int png_search_png_len = 707;
