Compile-Time Map - Static Hash Map at Compile Time
==================================================

.. image:: https://travis-ci.org/ChoppinBlockParty/ctm.svg?branch=master
   :target: https://travis-ci.org/ChoppinBlockParty/ctm

Allows to have a hash map built at compile time. The map could be used to perform runtime as well
as compile-time lookups.

Install
------

* C++-14 supporting compiler
* Header-only usage without any external dependencies, except the Standard Library.

How to
------

More examples could be found in ``tests/tests.cpp``.

.. code:: c++

    class JsonSerializable {
    public:
      virtual ~JsonSerializable() {}

      static JsonSerializable* createFromJson(JsonObject const& json_object);
    };

    class Holy : public JsonSerializable {
    public:
      static JsonSerializable* createFromJson(JsonObject const& json_object) {
        auto holy = new Holy;
        // Fill `holy` with data from `json_object`
        return holy;
      }
    };
    class Moly : public JsonSerializable {
    public:
      static JsonSerializable* createFromJson(JsonObject const& json_object) {
        auto moly = new Moly;
        // Fill `moly` with data from `json_object`
        return moly;
      }
    };
    class Miny : public JsonSerializable {
    public:
      static JsonSerializable* createFromJson(JsonObject const& json_object) {
        auto miny = new Miny;
        // Fill `miny` with data from `json_object`
        return miny;
      }
    };
    class Moe : public JsonSerializable {
    public:
      static JsonSerializable* createFromJson(JsonObject const& json_object) {
        auto moe = new Moe;
        // Fill `moe` with data from `json_object`
        return moe;
      }
    };

    JsonSerializable* JsonSerializable::createFromJson(JsonObject const& json_object) {
      constexpr auto spec = ctm::makeHashMapSpec(std::make_tuple("Holy", &Holy::createFromJson),
                                                 std::make_tuple("Moly", &Moly::createFromJson),
                                                 std::make_tuple("Miny", &Miny::createFromJson),
                                                 std::make_tuple("Moe", &Moe::createFromJson));
      static constexpr auto map = ctm::HashMap<decltype(spec),
                                               spec.maxBucketSize,
                                               spec.bucketCount,
                                               spec.elementCount>::make(spec);
      auto factory_function = map[json_object.at("type")];
      if (factory_function == nullptr)
        return nullptr;
      return factory_function(json_object);
    }
