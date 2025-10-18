
vehicleLabels = ['APC', 'Tank', 'Jeep', 'Artillery', 'Bike', 'Drone'];
let vehicleCounts = [12, 25, 9, 6, 15, 4];

const barchart = new Chart(
    document.getElementById('barChart'),
    // 下面是new Chart的第二个参数
    {
        type:'bar',  //指定要画什么图
        data:{   // 图表的数据内容
            labels:vehicleLabels,   //这些是 X 轴上显示的分类名称。
            datasets:[    //实际画什么
                {
                    label:"使用次数",  //这个是图例
                    data:vehicleCounts,   //具体的数据
                    borderWidth:1,   //柱状图的宽度
                    // 如果有多个lable，那么chart.js会自动配色
                    // backgroundColor 填充颜色
                    // borderColor边框颜色
                },
                // 如果有多个数据集，就用多个{}
                {
                    label:'测试',
                    data:[1,2,3,4,5,6],
                    borderWidth:1
                }
            ]
        },

        // 里面的第三个参数，是options,就是告诉chart如何画
        options:{
            responsive: true,    //让图表自动适应父容器的宽度（浏览器窗口变化时自动调整）
            maintainAspectRatio: false,        //允许高度自由拉伸（不固定比例）
            plugins:{
                title:{display:true, text:'各载具使用次数（示例）'},  //整个canvas的标题是显示或者不显示
                legend: { display: true }   // 是否显示图例,这里总控制，是否显示图例，也就是说明不同的颜色代表什么
            },
            interaction: { mode: 'nearest', intersect: true }, //鼠标靠近最近的数据点时显示提示
            scales: {
            x: { grid: { display: false } },
            y: { beginAtZero: true,   //让 Y 轴从 0 开始，而不是自动从最小值
                ticks: { stepSize: 5 } }  //控制刻度的间距，比如 0, 5, 10, 15...
            }
        }
    }

);