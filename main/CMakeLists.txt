idf_component_register(SRCS 
                        "main.c"
                        "./button/xiaozhi_button.c"
                        "./wifi_sta/xiaozhi_wifi_sta.c"
                      "./lcd/xiaozhi_lcd.c"
                      "./lvgl/xiaozhi_lvgl.c"
                      "./emoj/xiaozhi_emoj.c"
                    INCLUDE_DIRS 
                        "."
                        "./button"
                        "./wifi_sta"
                      "./lcd"
                      "./lvgl"
                      "./emoj"
                      EMBED_FILES image.jpg
                        )
