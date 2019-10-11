#ifndef HEADER_BASE_TIMETOOL
#define HEADER_BASE_TIMETOOL

#include <mutex>
namespace base {

    class CMemoryPool;
    class CTimeTool {
    public:
        CTimeTool();
        ~CTimeTool();
        CTimeTool(CTimeTool const& t);

        void Now();
        std::string GetDateStr();
        int GetDate();
        int GetYearDay();
        int GetMonthDay();
        int GetWeekDay();
        int GetMonth();
        int GetYear();
        int GetHour();
        int GetMin();
        int GetSec();
        time_t GetMsec();

        //return xxxx xx xx-xx:xx:xx
        std::string GetFormatTime();
        //return xxxx xx xx-xx:xx:xx
        bool GetFormatTime(char* res, int len);

        bool operator==(CTimeTool const& t);
        bool operator>(CTimeTool const& t);
        bool operator>=(CTimeTool const& t);
        bool operator<(CTimeTool const& t);
        bool operator<=(CTimeTool const& t);

    private:
        static time_t mktime64(unsigned int year, unsigned int mon,
                                   unsigned int day, unsigned int hour,
                                   unsigned int min, unsigned int sec);
        static int gmtime64(const time_t *t, std::tm *tp);

        void UpdateTm();

    private:
        bool          _update_time;  // need update tm struct
        time_t        _time;
        std::tm       _tm;
        std::mutex    _mutex;
    };
}

#endif
