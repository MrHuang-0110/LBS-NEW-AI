/**
  ******************************************************************************
  * @file    network_data_params.c
  * @author  AST Embedded Analytics Research Platform
  * @date    2026-03-24T15:40:01+0800
  * @brief   AI Tool Automatic Code Generator for Embedded NN computing
  ******************************************************************************
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  ******************************************************************************
  */

#include "network_data_params.h"


/**  Activations Section  ****************************************************/
ai_handle g_network_activations_table[1 + 2] = {
  AI_HANDLE_PTR(AI_MAGIC_MARKER),
  AI_HANDLE_PTR(NULL),
  AI_HANDLE_PTR(AI_MAGIC_MARKER),
};




/**  Weights Section  ********************************************************/
AI_ALIGNED(32)
const ai_u64 s_network_weights_array_u64[337] = {
  0xf87f28fe7f113781U, 0xe9fc487f81684701U, 0x8127abff608181dfU, 0xc8104f07ffe8189U,
  0x36e9027f81c2abebU, 0x2bff81d4477f1b81U, 0x2c817566815a7f57U, 0x7f28068181bdf5fbU,
  0x81c4fc2643816cd9U, 0x7367265e7f7f27caU, 0xce2e7fa301458181U, 0xd68198097fcbe781U,
  0x6812581248f81faU, 0x8ced3881da387f14U, 0xd581a91521818123U, 0x2c8109e781aa647fU,
  0x41ea127f2fd87fa2U, 0x81ca7f8542812cU, 0x147f14dc7f3ad87fU, 0x813b7fda48f581f7U,
  0xf12a81f17f891e16U, 0x387fe27fe0f8037fU, 0x3c810cb42309811eU, 0x7f67edea157f8128U,
  0xfffff976000009c4U, 0x897000002a0U, 0xfffff6f3U, 0x3fcU,
  0x7bf00000000U, 0xfffff988U, 0x9f10000129bU, 0xdd9fffff9d3U,
  0x238500001383U, 0x12fdU, 0xb5700000e27U, 0xfffffdf7U,
  0xfffffeed00000fb2U, 0x9bcffffee0fU, 0xffffe0a4U, 0xffffe4d7U,
  0xc3600000000U, 0xfffff912000014f6U, 0x121eU, 0xc3fU,
  0x163bU, 0x10cdfffff75fU, 0xc80000004adU, 0x200fU,
  0xe7900000897U, 0xa73fffffbcbU, 0xa3600001c22U, 0xffffe6a10000324aU,
  0x22cd0000031dU, 0xe35fffff6c8U, 0x6b800000000U, 0x3683000006b8U,
  0x912c16533348dda0U, 0x138ccecca51cf7aaU, 0xf308fa3dff1827b6U, 0x44c415d453accdbfU,
  0x912761e6b586160eU, 0xea4d7f4765361653U, 0x5100361cafd930a5U, 0xa14171dea5a9e9c4U,
  0xc9b5bdfb487b0fe1U, 0xb8568132381b68f7U, 0xd80dc9182c9245feU, 0xe221c0d2bb5c4a1cU,
  0xf891dd13685a0eeU, 0x3dfc7e3868fcce5eU, 0xe82cf3ed2915115cU, 0x45140f31e7d50134U,
  0x1a298e395cf1b772U, 0x5dfa45134f33c4fdU, 0x4433c9a06ee2c0ecU, 0x42f91e0bf04ddf87U,
  0xe62bff90c49e8135U, 0xf3c728e393c408a8U, 0x14c1efcce60b3d07U, 0xe31e194e0bbf8d40U,
  0x30deec2cf0095bb1U, 0x1346c03bf70e304bU, 0x49ba55e3c91b352cU, 0x304ff3fd8f2071cU,
  0xe2c6380f15adb32fU, 0xbe507ffe22f9ea6eU, 0x1dc2b5e0de235c4cU, 0x162d0c41b228ca5cU,
  0xdbe5b8f5123b27aaU, 0xc751816138e90d16U, 0xd1b220a6140a01f8U, 0xc5d0c2cf13fc4758U,
  0x1e9f0e131d5c02eU, 0xde4e5b21280f155bU, 0x2b09dc0dbcfe1424U, 0xd1555fed8f54957U,
  0x1fb2d2076e463c0dU, 0xaf1afc6c17fe45cbU, 0xde0e3ff3e6e1f82bU, 0xec001ec6f06a41f4U,
  0xcfecd8f11fdeef2cU, 0xfddc69fa69c0db3bU, 0xb1d6e522e654f21cU, 0xd8097f16eb31fb07U,
  0x9a9dcc048752136bU, 0x50c3a6c1375f84ddU, 0x6fef5bc462ed0b98U, 0x6d666e97e563f2bfU,
  0x32ccdf2d3316a176U, 0x358b9034e6f6bd59U, 0x3a8103d627f23ef7U, 0x902602f5f916f6ffU,
  0xf108231ceea5d108U, 0x38b2efbf44eaa122U, 0xef5f0c10c6302c2aU, 0xcb3cd4da6dcdf943U,
  0x41452c50b23abacfU, 0x2eb60dfedde1441dU, 0x45342419eadde9b3U, 0x1cdec2de53d11281U,
  0x1811c6256f3856d0U, 0xd628920def1b0fc0U, 0xbed729e91bb9da07U, 0xd71acd11a11743ddU,
  0xd7a039bf3ddca230U, 0x271e7f2f5e0eea59U, 0x2cf9a736db2b3a46U, 0x494aebd3110af002U,
  0x1cb54748712b2704U, 0xef41a5162637d9c8U, 0xbcb14f1903b01450U, 0xbac337f6ab090153U,
  0xf2a6cab45181d5cfU, 0x25395340f9f21c05U, 0x3904af32022d131bU, 0x74028cf08223d31U,
  0xfc1f304535d2ab6U, 0xf54a8168d80a6219U, 0x13c23cae01c31fe7U, 0xfe38fd05da622bf9U,
  0xbc07e49558edab25U, 0x8132e4223112ffaU, 0xe342b90db366412dU, 0x3ce87b28922300f1U,
  0x365ef3e8b5c9ac54U, 0x1fb5e2ce38c488c0U, 0x2220e624c9534f32U, 0xcfecdd0d73c9bf27U,
  0xfa24e075f953bf22U, 0xd4a60e81fecd0adcU, 0xff5976450adefcf0U, 0x1ff5e33125022fe0U,
  0xda6d2f2dd2b1ff6bU, 0x3007288946039244U, 0x22e6be211f544ef8U, 0x403ef9ef4fa2bf31U,
  0x3739cc0adb39e10dU, 0x240531b4e20144beU, 0xcf1a10334ef3b6ccU, 0x4610d8417feb48a5U,
  0x16d1dd37c73d24f2U, 0xdb0951fcd40ef11bU, 0xecf9e4c4ff1eb519U, 0xe2dc15d4efd9f60dU,
  0xf2daf3c433c24ae6U, 0xfb098fff3120f423U, 0x2681c6d6a711280cU, 0x111f211e4f0b71bU,
  0x2e66caf70ba8f961U, 0x4bd1bbeb27c68a17U, 0xe57135f02113d16U, 0x38e4f5f01dd7f1f6U,
  0x2561c54eb566f4e3U, 0x20f0f4bd04ee1a0eU, 0x3c0c08353081e5daU, 0x1ddeeffd2fd41692U,
  0x6d60fde053616daU, 0xcc34a550d83326feU, 0xfae319c0f8b0250fU, 0xc8e112f4b67e31efU,
  0xfc01c8a2f985abedU, 0x32107ff731e4073eU, 0xcc1fbf22f9222e12U, 0x4b4152c50529e828U,
  0xcf1af42622b5c84aU, 0x42b69ab11d34a011U, 0x17f3cb4fc95a3c0dU, 0xe40efe176bafe029U,
  0xe178fc43c34a0df7U, 0xfddae9940027f5dfU, 0x3b1a142357a7cfc1U, 0x46a7f2d167123b81U,
  0x3432e2c8dd9dca60U, 0x13d4d3b02fe1ee1fU, 0x404deb192b7f2828U, 0xf03213f730ca2022U,
  0x3222ed5bd072f0e4U, 0x38bdb6a1bffdbfd8U, 0x32f6b3022b3fad2U, 0x29b7c52a0403fcebU,
  0xe2b3d402545f0820U, 0xc3338129dc004becU, 0xdbe648d1f3a812feU, 0x43cb39d20230d6d6U,
  0x18922f11ffe4a20fU, 0x290176065a06c6e6U, 0x1e39b24cc9f90a15U, 0x1c5531001111f2deU,
  0xafffc0d4d9b0bfefU, 0x33140f103cd2e129U, 0x4c0df943095f451cU, 0x42d74eda51d51ffbU,
  0xe23187f0b70a1e7U, 0x3a3eabac0a12b317U, 0xd4f862c377949a06U, 0x21f739445db70bf8U,
  0xe2042ab5cd94d349U, 0x7cbbb1c1c7d9cb1aU, 0xe83dea65ef6a0af4U, 0xdedfd72a40c02025U,
  0x1704dc54bb7fc8eaU, 0xf802e610ccfbdf3eU, 0xff58582628c6abdbU, 0x17a4db374fd204c4U,
  0x3ea0a16f315f0fbU, 0xb62ffc52c017f7cbU, 0xc1fb0eaa34ff1f1dU, 0x38ecf52df5274805U,
  0x9a7db9cdfaf9ccfU, 0xd0187f4a2c2e3f5cU, 0xfae5c60c2a125d27U, 0x3d47f323bbf7d526U,
  0x5a1843fb3e7f9ea6U, 0xdec3cc736fb3a13eU, 0xd54fbfdbb4413cb6U, 0x6a966307d1d889dbU,
  0x16c36a969d0cebaaU, 0x329b2e3b924ac7faU, 0xfa9315eb1c63dbf6U, 0x90ce5d91475d26c5U,
  0xe1132cea3eeff3bdU, 0xfe3dce22cd0a4bf9U, 0xd0ff53b62fbe2214U, 0xd4f41ad7c6164bf7U,
  0xdcf627da12c59e33U, 0x32217f0c1e320c27U, 0xff0ec4f8f7f9fb03U, 0xfd315a25d02e1f22U,
  0xd1e933f45128111dU, 0xc605a830d315562eU, 0xfabc0cd0e2bd2741U, 0x19f729df0049ee0bU,
  0xfcc830b826b9ffeeU, 0xe307f3530c91740U, 0x2df6f738105a040bU, 0x1d4528ceac13182bU,
  0x34d00459f920ec19U, 0xcf62e521d9423ae8U, 0xc6e416b239c2ed1cU, 0x1517eb37b928f110U,
  0x35e70ef737dea4ceU, 0xe100662cfa2aec02U, 0x133d8aef254a1064U, 0x174e7fe5c44e1959U,
  0xf9e6014f5d1dedcdU, 0x5e6470006f5e0U, 0xcd732bce9e520e8U, 0xfdeffcf0c9374cfcU,
  0xf0329e439ca0818U, 0xeafd7f2c3a18033dU, 0x1acce014c718282fU, 0x2f606ae5c5fa2307U,
  0xdbbfc840591a0707U, 0xca36f40ad9ef4112U, 0xdbc70db838d14005U, 0x32e7f320fd1ef909U,
  0xde1c38ac0c819b23U, 0xd5152a4123f110f5U, 0x20fb8f7b54e151aU, 0x4f2c7ff8c266264fU,
  0x484a44b120fed94eU, 0x7fb5eeca142dbaedU, 0xf05abd55c65b4bf2U, 0x4421ed2c2fb501f9U,
  0xd432f52ead0f32bfU, 0xdef3e2d6fecbd707U, 0x1e5f1af4ecc39a13U, 0x5b231d6580631dfU,
  0xe5580bfff48fb821U, 0x29c8b8eaec25c132U, 0x1016eb201a0950dbU, 0x16f8ec0619dc2de4U,
  0xf3591c1ce57ac3ccU, 0x270ff6cbc9f2ecf9U, 0x273e6efb4ff49002U, 0x2be717307fcc34e9U,
  0x83823dcb9c1cd5cU, 0x7dba23f324419bdfU, 0x3b0b1a2b1c1d2630U, 0xda263fed6e012df3U,
  0xef2d3035067fed28U, 0xe3dfd1e0cbeadef9U, 0xd02344fd09db8eabU, 0x33bef0d276eb26d5U,
  0xd1fcca411af44e0fU, 0xe50ae85af61348efU, 0xe817e0f7e4e21907U, 0xcce8e0c49f3f02efU,
  0xcdd1e5cf21bfd036U, 0xe4497f274a10d2e7U, 0x32edd80a06040f1fU, 0x253907c6be0322f2U,
  0x1358fffffe44U, 0x1099ffffeafdU, 0x1c6f000015d4U, 0x1a94ffffe96dU,
  0x12d10000196aU, 0x7610000231bU, 0xfffffbcb0000192cU, 0x1fac00001e19U,
  0x153100002c7fU, 0xea60000149bU, 0x118200002e23U, 0x1a32000005c6U,
  0x220200001063U, 0x1be200001984U, 0x1f1d00001b10U, 0x1798000016e5U,
  0x701beef39cf4cdb6U, 0x836e3a7ff4dca2e9U, 0x9102a27b7fb76656U, 0x941c557ebd0efab7U,
  0xa4c7205c35351330U, 0xd8a3b9af952d4d4bU, 0x142c2fd6ac3f96b5U, 0x3785b3814b5e4e42U,
  0x31cfffffc8cU,
};


ai_handle g_network_weights_table[1 + 2] = {
  AI_HANDLE_PTR(AI_MAGIC_MARKER),
  AI_HANDLE_PTR(s_network_weights_array_u64),
  AI_HANDLE_PTR(AI_MAGIC_MARKER),
};

