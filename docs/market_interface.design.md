# Market Interface Design

Interface each market should implement.

## Interface

```cpp
struct Currency_pair {
    std::string base;
    std::string quote;
};

struct Price {
    std::string value;
    Currency_pair pair;
};

enum class Period {
    Last_hour,
    Last_quarter_day,
    Last_half_day,
    Last_day,
    Last_week,
    Last_month,
    Last_year,
    All
};

struct Candle {
    std::chrono::time_point start_time;
    std::chrono::duration<> duration;
    std::string low;
    std::string high;
    std::string opening;
    std::string closing;
    std::string volume;
};

class Market {
   public:
    /// Return list of supported currency pairs, makes html request if not
    /// initialized yet.
    auto currency_pairs() -> std::vector<Currency_pair>;

    /// Return the price of the given Currency_pair at the time point.
    auto price_at(Currency_pair, Candle::Time_point_t) -> Price;

    /// Returns the opening price(UTC midnight) of the Currency_pair.
    auto opening_price(Currency_pair) -> Price;

    /// Return historic data for a given time period and Currency_pair. Values
    /// are not cached. Granularity is implementation defined.
    auto candles(Currency_pair, Period) -> std::vector<Candle>;

    /// Starts connection to websocket if not started yet. Registers to start
    /// receiving live price data from read() for Currency_pair.
    void subscribe(Currency_pair);

    /// Starts websocket connection if not started yet, listens for messages,
    /// blocking until live price is received.
    auto read() -> Price;
};
```
