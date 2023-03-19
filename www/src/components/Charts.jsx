import { useEffect, useRef, useState } from 'react'
import { ArrowPathIcon } from '@heroicons/react/24/outline'
import {
  Chart,
  CategoryScale,
  LinearScale,
  PointElement,
  LineElement,
  Legend,
  Title,
  Tooltip,
  TimeScale,
} from 'chart.js'
import { Line } from 'react-chartjs-2'
import { loadFileData, loadFilesList } from '../api'
import { CHART_DATA_TEMPLATE, getChartOptions } from '../helpers'

const Header = () => (
  <h2 className="text-center text-lg font-semibold">Temperature & Humidity</h2>
)

const Charts = () => {
  const [isLoading, setIsLoading] = useState(true)
  const [files, setFiles] = useState([])
  const [currentFileIndex, setCurrentFileIndex] = useState(null)
  const [chartData, setChartData] = useState(CHART_DATA_TEMPLATE)
  const [chartOptions, setChartOptions] = useState({})
  const [chartWidth, setChartWidth] = useState('100%')

  const chartContainer = useRef(null)
  const chartRef = useRef(null)

  useEffect(() => {
    async function loadFileListAndPrepare() {
      try {
        const files = await loadFilesList('http://greenhouse.local/list')
        setFiles(files)
        setCurrentFileIndex(0)
      } catch (error) {
        console.error(error)
      } finally {
        setIsLoading(false)
      }
    }
    loadFileListAndPrepare()
  }, [])

  useEffect(() => {
    async function loadFileDataAndPrepare() {
      if (currentFileIndex == null || !files[currentFileIndex]) {
        return
      }

      setIsLoading(true)
      try {
        const jsonData = await loadFileData(
          `http://greenhouse.local/data/${files[currentFileIndex]}`,
        )

        const chartData = jsonData.reduce(
          (data, { time, temperature, humidity }) => {
            data.labels.push(time)
            data.datasets[0].data.push({ x: time, y: temperature })
            data.datasets[1].data.push({ x: time, y: humidity })
            return data
          },
          CHART_DATA_TEMPLATE,
        )
        chartData.xAxisTitle = new Date(
          files[currentFileIndex].split('.')[0],
        ).toLocaleDateString()

        setChartData(chartData)
        setChartOptions(getChartOptions(chartData.xAxisTitle, chartData.labels))
        setChartWidth(
          chartData.labels.length > 15
            ? chartData.labels.length * 30 + 'px'
            : '100%',
        )

        chartRef.current.update()
      } catch (error) {
        console.error(error)
      } finally {
        setIsLoading(false)
      }
    }
    loadFileDataAndPrepare()
  }, [currentFileIndex])

  // Scroll chart to the very right once chart data loaded
  useEffect(() => {
    if (chartContainer.current) {
      chartContainer.current.scrollLeft = chartContainer.current.scrollWidth
    }
  }, [chartData.labels.length])

  Chart.register(
    CategoryScale,
    LinearScale,
    TimeScale,
    Legend,
    Title,
    Tooltip,
    PointElement,
    LineElement,
  )

  if (isLoading) {
    return (
      <section>
        <Header />
        <div className="mt-10">
          <ArrowPathIcon className="mx-auto h-8 w-8 animate-spin text-gray-500" />
        </div>
      </section>
    )
  }

  if (!files[currentFileIndex].length) {
    return (
      <section>
        <Header />
        <p className="mt-6 text-center italic text-gray-500">Data not found</p>
      </section>
    )
  }

  return (
    <section>
      <Header />
      <div ref={chartContainer} className="w-full overflow-x-auto">
        <div className="mx-auto h-96" style={{ width: chartWidth }}>
          <Line ref={chartRef} data={chartData} options={chartOptions} />
        </div>
      </div>
    </section>
  )
}

export default Charts
