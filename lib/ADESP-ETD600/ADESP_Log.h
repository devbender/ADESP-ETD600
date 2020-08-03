
#define LOG_ENABLE

// LOGGING -----------------------------------------------------------

#ifdef LOG_ENABLE

#define LOG_INFO(MSG)       //Serial.printf(">> INFO:     %s  \n", MSG);
#define LOG_WARNING(MSG) 	Serial.printf(">> WARNING:  %s  \n", MSG);
#define LOG_ERROR(MSG) 		Serial.printf(">> ERROR:    %s  \n", MSG);
#define SYS_MSG(MSG)        Serial.println(MSG);
#define SYS_MSG2(MSG)       Serial.print(MSG);
#define SYS_MSGL(LBL, MSG)	Serial.printf("%s %s\n", LBL, MSG);

#define LOG_1VAR(LABEL, VAR)		Serial.print(LABEL); Serial.print(" : "); Serial.println(VAR);

#define LOG_2VAR(LABEL, VAR1, VAR2)	Serial.print(LABEL); Serial.print(" : "); Serial.print(VAR1); \
									Serial.print(" | "); Serial.println(VAR2);

#define LOG_3VAR(LABEL, VAR1, VAR2, VAR3) \
									Serial.print(LABEL); Serial.print(" : "); \
									Serial.print(VAR1);  Serial.print(" | "); \
									Serial.print(VAR2); Serial.print(" | "); \
									Serial.println(VAR3);
#endif