import { HeartIcon } from '@heroicons/react/24/solid'
import Charts from './components/Charts'

export default function App() {
  return (
    <div className="min-h-full">
      <nav className="border-b border-gray-200 bg-white dark:border-gray-700 dark:bg-gray-800">
        <div className="mx-auto max-w-7xl px-4 sm:px-6 lg:px-8">
          <div className="flex h-16 items-center justify-center gap-2">
            <HeartIcon className="h-8 w-8 text-emerald-500" />
            <h1 className="text-lg font-semibold">Greenhouse Monitoring</h1>
          </div>
        </div>
      </nav>

      <main>
        <div className="mx-auto max-w-7xl py-6 sm:px-6 lg:px-8">
          <Charts />
        </div>
      </main>
    </div>
  )
}
