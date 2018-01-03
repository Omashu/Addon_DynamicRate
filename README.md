### Addon For YorozuyaGS
https://github.com/goodwinxp/Yorozuya

### Configure `global.json`

```
{
	"name" : "addon.dynamic_rate",
	"config" : {
	  "activated" : true,
	  "flush_logs" : false,
	  "config_path" : "./YorozuyaGS/dynamic_rate.json",
	  "max_lv" : 65,
	  "base_default_exp_rate" : 1.0, // from initial
	  "base_premium_exp_rate" : 1.0  // from initial
	}
}
```

### Configure `dynamic_rate.json`

```
[
	{
		"lv" : 1,
		"default_exp_rate" : 100.0,
		"premium_exp_rate" : 200.0
	},
	{
		"lv" : 2,
		"default_exp_rate" : 50.0,
		"premium_exp_rate" : 100.0
	}
	...
]
```


### Additional
* https://github.com/goodwinxp/Yorozuya
* https://github.com/gabime/spdlog